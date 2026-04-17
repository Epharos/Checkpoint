#include <Common/Core/Macros.hpp>
#include <Common/Core/Log.hpp>
#include <Common/Async/JobSystem.hpp>
#include <Common/Plugin/PluginHost.hpp>

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>

#include <VulkanRHI.hpp> // TMP

#include <GLFWWindow.hpp> // TMP

#include <Resources/AssetRegistry.hpp>
#include "../../../Rendering/Private/Renderer.hpp"

#include <filesystem>

int main(int argc, char** argv)
{
    ////////////////////////////
    /// Setup Loggers
    ////////////////////////////
    constexpr auto InitLabel = "Init";
    constexpr auto CleanupLabel = "Cleanup";

    const std::shared_ptr<cp::IMessageVisitor> messageVisitor = std::make_shared<cp::ConsoleVisitor>();
    const std::shared_ptr<cp::IMessageVisitor> fileMessageVisitor = std::make_shared<cp::FileVisitor>();

    const std::shared_ptr<cp::ILogger> logger = std::make_shared<cp::ConsoleLogger>(messageVisitor);
    const std::shared_ptr<cp::ILogger> fileLogger = std::make_shared<cp::FileLogger>(fileMessageVisitor, "log.txt");
    const auto compositeLogger = std::make_shared<cp::CompositeLogger>();
    compositeLogger->AddLogger(logger);
    compositeLogger->AddLogger(fileLogger);

    compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Info, InitLabel, cp::Message::Create("Hello, World!")));

    cp::RegistryManager registryManager;
    registryManager.GetOrCreate<cp::IRenderPass>("Renderpass");

    ////////////////////////////
    /// Load Plugins
    ////////////////////////////

    cp::PluginHostContext pluginHostContext
    {
        .mainLogger = compositeLogger.get(),
        .registryManager = &registryManager,
    };

    cp::PluginHost pluginHost { pluginHostContext };

    const std::filesystem::path executableDirectory =
        (argc > 0 && argv[0] != nullptr)
        ? std::filesystem::absolute(argv[0]).parent_path()
        : std::filesystem::current_path();
    const std::filesystem::path pluginsDirectory = executableDirectory / "Plugins";

    if (std::filesystem::exists(pluginsDirectory) && std::filesystem::is_directory(pluginsDirectory))
    {
        const size_t loadedPluginCount = pluginHost.LoadPluginsFromDirectory(pluginsDirectory);
        if (loadedPluginCount > 0)
        {
            compositeLogger->Log(CP_LOG_EVENT(
                cp::ILogger::Info,
                InitLabel,
                cp::Message::Create("Loaded {} plugin(s) from {}", loadedPluginCount, pluginsDirectory.string())
            ));
        }
        else
        {
            const std::string& pluginError = pluginHost.GetLastError();
            if (!pluginError.empty())
            {
                compositeLogger->Log(CP_LOG_EVENT(
                    cp::ILogger::Warning,
                    InitLabel,
                    cp::Message::Create("No plugin loaded from {} ({})", pluginsDirectory.string(), pluginError)
                ));
            }
            else
            {
                compositeLogger->Log(CP_LOG_EVENT(
                    cp::ILogger::Info,
                    InitLabel,
                    cp::Message::Create("No plugins found in {}", pluginsDirectory.string())
                ));
            }
        }
    }
    else
    {
        compositeLogger->Log(CP_LOG_EVENT(
            cp::ILogger::Info,
            InitLabel,
            cp::Message::Create("Plugin directory not found: {}", pluginsDirectory.string())
        ));
    }

    ////////////////////////////
    /// Initialize JobSystem
    ////////////////////////////
    cp::JobSystem::Initialize(std::thread::hardware_concurrency() / 2);

    compositeLogger->Log(CP_LOG_EVENT(
        cp::ILogger::Info,
        InitLabel,
        cp::Message::Create("Job System initialized with {} workers", cp::JobSystem::GetInstance().GetWorkerCount())
    ));

    ////////////////////////////
    /// Setup RHI (Instance, Devices)
    ////////////////////////////

    const std::unique_ptr<cp::RenderingHardwareInterface> rhi = std::make_unique<cp::VulkanRHI>(compositeLogger);

    cp::InstanceInfo instanceInfo
    {
        .appName = "TestApp",
        .appVersion = CP_MAKE_VERSION(0, 1, 0, 0),
        .enableValidationLayers = true
    };

    cp::IInstance& rhiInstance = rhi->CreateInstance(instanceInfo);
    cp::IPhysicalDevice& physicalDevice = rhi->CreatePhysicalDevice();
    cp::IDevice& device = rhi->CreateDevice();

    ////////////////////////////
    /// Initialize Asset Registry
    ////////////////////////////

    cp::AssetRegistry::Instance().Initialize(*rhi);

    compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Info, InitLabel, cp::Message::Create("Asset Registry initialized")));

    ////////////////////////////
    /// Setup Window
    ////////////////////////////

    cp::WindowInfo windowInfo
    {
        .title = "Test Window",
        .extent = cp::Extent2D{ 800, 600 },
        .resizable = true
    };

    cp::GLFWWindow window { windowInfo };

    ////////////////////////////
    /// Setup Renderer
    ////////////////////////////

    cp::RendererInfo renderInfo
    {
        .frameCount = 3,
        .extent = windowInfo.extent,
        .imageFormat = cp::Format::R8G8B8A8_UNORM,
        .nativeWindowHandle = window.GetNativeWindowHandle(),
        .registryManager = &registryManager
    };

    cp::Renderer renderer { renderInfo, *rhi };

    renderer.AddFrameGraphPass("SceneRenderPass");
    renderer.AddFrameGraphPass("NegativePostFX");
    renderer.RecompileFrameGraph();

    ////////////////////////////
    /// Loop
    ////////////////////////////

    while (!window.ShouldClose())
    {
        window.PollEvents();

        renderer.BeginFrame();
        renderer.Render();
        renderer.EndFrame();
    }

    ////////////////////////////
    /// Cleanup
    ////////////////////////////

    renderer.ResetFrameGraph();
    cp::AssetRegistry::Instance().Cleanup();

    compositeLogger->Log(CP_LOG_EVENT(
        cp::ILogger::Info,
        CleanupLabel,
        cp::Message::Create("Waiting for background jobs to complete...")
    ));

    cp::JobSystem::GetInstance().Wait();

    compositeLogger->Log(CP_LOG_EVENT(
        cp::ILogger::Info,
        CleanupLabel,
        cp::Message::Create("All background jobs completed")
    ));

    pluginHost.UnloadAll();
    compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Info, CleanupLabel, cp::Message::Create("Plugins unloaded")));
    
    cp::JobSystem::Shutdown();
    compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Info, CleanupLabel, cp::Message::Create("JobSystem shut down")));

    return 0;
}

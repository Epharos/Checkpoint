#include <Common/Core/Macros.hpp>
#include <Common/Core/Log.hpp>
#include <Common/Async/JobSystem.hpp>

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>

#include <VulkanRHI.hpp> // TMP

#include <GLFWWindow.hpp> // TMP

#include "../../../Rendering/Private/Renderer.hpp"

int main(int argc, char** argv)
{
    ////////////////////////////
    /// Setup Loggers
    ////////////////////////////
    constexpr const char* InitLabel = "Init";
    constexpr const char* CleanupLabel = "Cleanup";

    const std::shared_ptr<cp::IMessageVisitor> messageVisitor = std::make_shared<cp::ConsoleVisitor>();
    const std::shared_ptr<cp::IMessageVisitor> fileMessageVisitor = std::make_shared<cp::FileVisitor>();

    const std::shared_ptr<cp::ILogger> logger = std::make_shared<cp::ConsoleLogger>(messageVisitor);
    const std::shared_ptr<cp::ILogger> fileLogger = std::make_shared<cp::FileLogger>(fileMessageVisitor, "log.txt");
    const auto compositeLogger = std::make_shared<cp::CompositeLogger>();
    compositeLogger->AddLogger(logger);
    compositeLogger->AddLogger(fileLogger);

    compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Info, InitLabel, cp::Message::Create("Hello, World!")));

    ////////////////////////////
    /// Initialize JobSystem
    ////////////////////////////
    cp::JobSystem::Initialize(std::thread::hardware_concurrency() / 2);

    compositeLogger->Log(CP_LOG_EVENT(
        cp::ILogger::Info,
        InitLabel,
        cp::Message::Create("Job system initialized with {} workers", cp::JobSystem::GetInstance().GetWorkerCount())
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
        .nativeWindowHandle = window.GetNativeWindowHandle()
    };

    cp::Renderer renderer { renderInfo, *rhi };

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
    
    cp::JobSystem::Shutdown();
    compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Info, CleanupLabel, cp::Message::Create("JobSystem shut down")));

    return 0;
}

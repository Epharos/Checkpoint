#include <Common/Core/Macros.hpp>
#include <Common/Core/Log.hpp>
#include <Common/Async/JobSystem.hpp>
#include <Common/IO/FileHelper.hpp>
#include <Common/Plugin/PluginHost.hpp>

#include <ECS/ECS.hpp>

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>

#include <VulkanRHI.hpp> // TMP

#include <GLFWWindow.hpp> // TMP
#include "../../../ExamplePlugin/Private/Components/MovementComponents.hpp"

#include <Resources/AssetRegistry.hpp>
#include "../../../Rendering/Private/Renderer.hpp"

#include <filesystem>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <memory>
#include <span>
#include <vector>

namespace
{
    class MemorySerializer final : public cp::ISerializer
    {
    public:
        void Write(const std::span<const std::byte> _src) override
        {
            bytes.insert(bytes.end(), _src.begin(), _src.end());
        }

        std::vector<std::byte> bytes;
    };

    class MemoryDeserializer final : public cp::IDeserializer
    {
    public:
        explicit MemoryDeserializer(const std::vector<uint8_t>& _bytes)
            : bytes(_bytes)
        {
        }

        [[nodiscard]] bool Read(const std::span<std::byte> _dst) override
        {
            if (cursor + _dst.size() > bytes.size())
            {
                return false;
            }

            std::memcpy(_dst.data(), bytes.data() + cursor, _dst.size());
            cursor += _dst.size();
            return true;
        }

    private:
        const std::vector<uint8_t>& bytes;
        size_t cursor = 0;
    };

    bool SaveBinaryFile(const std::filesystem::path& _path, const std::vector<std::byte>& _bytes)
    {
        std::ofstream file(_path, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            return false;
        }

        if (!_bytes.empty())
        {
            file.write(reinterpret_cast<const char*>(_bytes.data()), static_cast<std::streamsize>(_bytes.size()));
        }

        return file.good();
    }
}

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
    registryManager.GetOrCreate<cp::ecs::ISystem>("EcsSystem");
    registryManager.GetOrCreate<cp::ecs::IComponentRegistrar>("EcsComponent");

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
    /// Load ECS world
    ////////////////////////////

    cp::ecs::World ecsWorld;
    cp::ecs::CommandBuffer ecsCommandBuffer;
    std::vector<std::unique_ptr<cp::ecs::ISystem>> ecsSystems;

    const auto registerPluginComponents = [&registryManager](cp::ecs::World& _world)
    {
        if (cp::Registry<cp::ecs::IComponentRegistrar>* componentRegistry =
            registryManager.Find<cp::ecs::IComponentRegistrar>("EcsComponent"))
        {
            for (const std::string& registrarName : componentRegistry->Names())
            {
                std::unique_ptr<cp::ecs::IComponentRegistrar> registrar = componentRegistry->Create(registrarName);
                registrar->Register(_world);
            }
        }
    };

    // Bootstrap a basic ECS scene then persist it as Scene.ecsbin.
    {
    //     cp::ecs::World bootstrapWorld;
    //     registerPluginComponents(bootstrapWorld);
    //     bootstrapWorld.SetRequiredPlugins({ "ExamplePlugin" });
    //     bootstrapWorld.SetStartupSystems({ cp::ecs::MakeTypeGuid("ExamplePlugin.VelocityMovementSystem") });
    //
    //     for (uint32_t i = 0; i < 10; ++i)
    //     {
    //         const cp::ecs::Entity entity = bootstrapWorld.CreateEntity();
    //
    //         const float base = static_cast<float>(i);
    //         bootstrapWorld.AddComponent<cp::Position3D>(entity, cp::Position3D{
    //             .x = base * 1.5f,
    //             .y = base * 0.25f,
    //             .z = -base * 0.5f
    //         });
    //         bootstrapWorld.AddComponent<cp::Velocity3D>(entity, cp::Velocity3D{
    //             .x = 0.1f * (base + 1.0f),
    //             .y = 0.05f * (base + 1.0f),
    //             .z = 0.02f * (base + 1.0f)
    //         });
    //     }
    //
    //     MemorySerializer serializer;
    //     if (bootstrapWorld.SerializeBinary(serializer))
    //     {
    //         const std::filesystem::path outputPath = std::filesystem::current_path() / "Scene.ecsbin";
    //         if (!SaveBinaryFile(outputPath, serializer.bytes))
    //         {
    //             compositeLogger->Log(CP_LOG_EVENT(
    //                 cp::ILogger::Warning,
    //                 InitLabel,
    //                 cp::Message::Create("Failed to write ECS bootstrap scene '{}'", outputPath.string())
    //             ));
    //         }
    //     }
    //     else
    //     {
    //         compositeLogger->Log(CP_LOG_EVENT(
    //             cp::ILogger::Warning,
    //             InitLabel,
    //             cp::Message::Create("Failed to serialize ECS bootstrap scene")
    //         ));
    //     }
    }

    registerPluginComponents(ecsWorld);

    const std::filesystem::path worldPath = cp::FindFileInParentTree("Scene.ecsbin");
    if (worldPath.empty())
    {
        compositeLogger->Log(CP_LOG_EVENT(
            cp::ILogger::Warning,
            InitLabel,
            cp::Message::Create("ECS world file not found: Scene.ecsbin")
        ));
    }
    else
    {
        const std::vector<uint8_t> worldBytes = cp::LoadBinaryFile(worldPath);
        if (worldBytes.empty())
        {
            compositeLogger->Log(CP_LOG_EVENT(
                cp::ILogger::Warning,
                InitLabel,
                cp::Message::Create("ECS world file is empty: {}", worldPath.string())
            ));
        }
        else
        {
            MemoryDeserializer deserializer(worldBytes);
            if (!ecsWorld.DeserializeBinary(deserializer))
            {
                compositeLogger->Log(CP_LOG_EVENT(
                    cp::ILogger::Warning,
                    InitLabel,
                    cp::Message::Create("Failed to deserialize ECS world '{}'", worldPath.string())
                ));
            }
            else
            {
                const std::vector<std::string> loadedPluginNames = pluginHost.GetLoadedPluginNames();
                for (const std::string& requiredPlugin : ecsWorld.GetRequiredPlugins())
                {
                    const bool isLoaded = std::find(loadedPluginNames.begin(), loadedPluginNames.end(), requiredPlugin) != loadedPluginNames.end();
                    if (!isLoaded)
                    {
                        compositeLogger->Log(CP_LOG_EVENT(
                            cp::ILogger::Warning,
                            InitLabel,
                            cp::Message::Create("ECS world requires missing plugin '{}'", requiredPlugin)
                        ));
                    }
                }

                if (cp::Registry<cp::ecs::ISystem>* ecsSystemRegistry =
                    registryManager.Find<cp::ecs::ISystem>("EcsSystem"))
                {
                    for (const cp::ecs::TypeGuid systemGuid : ecsWorld.GetStartupSystems())
                    {
                        const std::string systemKey = cp::ecs::GuidToRegistryKey(systemGuid);
                        if (!ecsSystemRegistry->Contains(systemKey))
                        {
                            compositeLogger->Log(CP_LOG_EVENT(
                                cp::ILogger::Warning,
                                InitLabel,
                                cp::Message::Create("Missing ECS system for guid {}", systemKey)
                            ));
                            continue;
                        }

                        ecsSystems.push_back(ecsSystemRegistry->Create(systemKey));
                    }
                }
            }
        }
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
        .registryManager = &registryManager,
    };

    cp::Renderer renderer { renderInfo, *rhi };

    renderer.AddFrameGraphPass("OpaqueMaterialPass");
    renderer.AddFrameGraphPass("NegativePostFX");
    renderer.RecompileFrameGraph();

    ////////////////////////////
    /// Loop
    ////////////////////////////

    while (!window.ShouldClose())
    {
        window.PollEvents();

        for (const std::unique_ptr<cp::ecs::ISystem>& system : ecsSystems)
        {
            constexpr float FixedDeltaTime = 1.0f / 60.0f;
            system->Run(ecsWorld, ecsCommandBuffer, FixedDeltaTime);
        }
        ecsCommandBuffer.Playback(ecsWorld);

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

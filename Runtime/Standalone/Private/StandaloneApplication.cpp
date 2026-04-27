#include <StandaloneApplication.hpp>

#include <Common/Async/JobSystem.hpp>
#include <Common/Core/Macros.hpp>
#include <Common/IO/FileHelper.hpp>
#include <Common/Util/Clock.hpp>

#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

#include <Resources/AssetRegistry.hpp>
#include <Resources/Mesh.hpp>

#include <Rendering/Renderer.hpp>

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <span>
#include <thread>
#include <vector>

#include "../../../ExamplePlugin/Private/Components/RenderingComponentRegistrars.hpp"

namespace
{
    constexpr auto InitLabel = "Init";
    constexpr auto CleanupLabel = "Cleanup";

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

namespace cp
{
    StandaloneApplication::~StandaloneApplication() = default;

    bool StandaloneApplication::Init(int argc, char** argv)
    {
        InitLoggers();
        InitRegistry();
        InitPlugins(argc, argv);
        RegisterPluginComponents();
        InitJobSystem();
        InitRenderingHardwareInterface();
        InitAssetRegistry();
        InitEcsWorld();
        InitWindow();
        InitRenderer();

        initialized = true;
        return true;
    }

    void StandaloneApplication::Run()
    {
        if (!initialized || window == nullptr || renderer == nullptr)
        {
            return;
        }

        Clock deltaTimeClock;

        while (!window->ShouldClose())
        {
            window->PollEvents();

            for (const std::unique_ptr<ecs::ISystem>& system : ecsSystems)
            {
                system->Run(ecsWorld, ecsCommandBuffer, static_cast<float>(deltaTimeClock.Restart()));
            }

            ecsCommandBuffer.Playback(ecsWorld);

            renderer->BeginFrame();
            renderer->Render();
            renderer->EndFrame();
        }
    }

    void StandaloneApplication::Clear()
    {
        ecsSystems.clear();
        ecsWorld.Clear();
        ecsWorld.ClearComponentTypes();

        if (renderer != nullptr)
        {
            renderer->ResetFrameGraph();
            renderer.reset();
        }
        window.reset();
        rhi.reset();

        if (assetRegistryInitialized)
        {
            AssetRegistry::Instance().Cleanup();
            assetRegistryInitialized = false;
        }

        if (jobSystemInitialized)
        {
            JobSystem::GetInstance().Wait();
        }

        if (pluginHost != nullptr)
        {
            pluginHost->UnloadAll();
            if (compositeLogger != nullptr)
            {
                compositeLogger->Log(CP_LOG_EVENT(ILogger::Info, CleanupLabel, Message::Create("Plugins unloaded")));
            }
            pluginHost.reset();
        }

        if (jobSystemInitialized)
        {
            JobSystem::Shutdown();
            jobSystemInitialized = false;

            if (compositeLogger != nullptr)
            {
                compositeLogger->Log(CP_LOG_EVENT(ILogger::Info, CleanupLabel, Message::Create("JobSystem shut down")));
            }
        }

        initialized = false;
    }

    void StandaloneApplication::InitLoggers()
    {
        const std::shared_ptr<IMessageVisitor> messageVisitor = std::make_shared<ConsoleVisitor>();
        const std::shared_ptr<IMessageVisitor> fileMessageVisitor = std::make_shared<FileVisitor>();
        const std::shared_ptr<ILogger> logger = std::make_shared<ConsoleLogger>(messageVisitor);
        const std::shared_ptr<ILogger> fileLogger = std::make_shared<FileLogger>(fileMessageVisitor, "log.txt");

        const auto createdCompositeLogger = std::make_shared<CompositeLogger>();
        createdCompositeLogger->AddLogger(logger);
        createdCompositeLogger->AddLogger(fileLogger);
        compositeLogger = createdCompositeLogger;

        compositeLogger->Log(CP_LOG_EVENT(ILogger::Info, InitLabel, Message::Create("Hello, World!")));
    }

    void StandaloneApplication::InitRegistry()
    {
        registryManager.GetOrCreate<IRenderPass>("Renderpass");
        registryManager.GetOrCreate<ecs::ISystem>("EcsSystem");
        registryManager.GetOrCreate<ecs::IComponentRegistrar>("EcsComponent");
    }

    void StandaloneApplication::InitPlugins(int argc, char** argv)
    {
        PluginHostContext pluginHostContext
        {
            .mainLogger = compositeLogger.get(),
            .registryManager = &registryManager,
            .assetRegistry = &AssetRegistry::Instance(),
        };

        pluginHost = std::make_unique<PluginHost>(pluginHostContext);

        const std::filesystem::path executableDirectory =
            (argc > 0 && argv[0] != nullptr)
                ? std::filesystem::absolute(argv[0]).parent_path()
                : std::filesystem::current_path();
        const std::filesystem::path pluginsDirectory = executableDirectory / "Plugins";

        if (std::filesystem::exists(pluginsDirectory) && std::filesystem::is_directory(pluginsDirectory))
        {
            const size_t loadedPluginCount = pluginHost->LoadPluginsFromDirectory(pluginsDirectory);
            if (loadedPluginCount > 0)
            {
                compositeLogger->Log(CP_LOG_EVENT(
                    ILogger::Info,
                    InitLabel,
                    Message::Create("Loaded {} plugin(s) from {}", loadedPluginCount, pluginsDirectory.string())
                ));
            }
            else
            {
                const std::string& pluginError = pluginHost->GetLastError();
                if (!pluginError.empty())
                {
                    compositeLogger->Log(CP_LOG_EVENT(
                        ILogger::Warning,
                        InitLabel,
                        Message::Create("No plugin loaded from {} ({})", pluginsDirectory.string(), pluginError)
                    ));
                }
                else
                {
                    compositeLogger->Log(CP_LOG_EVENT(
                        ILogger::Info,
                        InitLabel,
                        Message::Create("No plugins found in {}", pluginsDirectory.string())
                    ));
                }
            }
        }
        else
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Info,
                InitLabel,
                Message::Create("Plugin directory not found: {}", pluginsDirectory.string())
            ));
        }
    }

    void StandaloneApplication::InitJobSystem()
    {
        JobSystem::Initialize(std::thread::hardware_concurrency() / 2);
        jobSystemInitialized = true;
        compositeLogger->Log(CP_LOG_EVENT(
            ILogger::Info,
            InitLabel,
            Message::Create("Job System initialized with {} workers", JobSystem::GetInstance().GetWorkerCount())
        ));
    }

    void StandaloneApplication::InitRenderingHardwareInterface()
    {
        rhi = std::make_unique<VulkanRHI>(compositeLogger);

        InstanceInfo instanceInfo
        {
            .appName = "TestApp",
            .appVersion = CP_MAKE_VERSION(0, 1, 0, 0),
            .enableValidationLayers = false
        };

        rhi->CreateInstance(instanceInfo);
        rhi->CreatePhysicalDevice();
        rhi->CreateDevice();
    }

    void StandaloneApplication::InitAssetRegistry()
    {
        AssetRegistry::Instance().Initialize(*rhi);
        assetRegistryInitialized = true;
        compositeLogger->Log(CP_LOG_EVENT(ILogger::Info, InitLabel, Message::Create("Asset Registry initialized")));
    }

    void StandaloneApplication::InitWindow()
    {
        WindowInfo windowInfo
        {
            .title = "Test Window",
            .extent = Extent2D{ 800, 600 },
            .resizable = true
        };

        window = std::make_unique<GLFWWindow>(windowInfo);
    }

    void StandaloneApplication::InitRenderer()
    {
        RendererInfo renderInfo
        {
            .frameCount = 3,
            .extent = Extent2D{ 800, 600 },
            .imageFormat = Format::R8G8B8A8_UNORM,
            .nativeWindowHandle = window->GetNativeWindowHandle(),
            .registryManager = &registryManager,
            .ecsWorld = &ecsWorld
        };

        renderer = std::make_unique<Renderer>(renderInfo, *rhi);
        renderer->AddFrameGraphPass("OpaqueMaterialPass");
        renderer->RecompileFrameGraph();
    }

    void StandaloneApplication::RegisterPluginComponents()
    {
        if (const Registry<ecs::IComponentRegistrar>* componentRegistry =
            registryManager.Find<ecs::IComponentRegistrar>("EcsComponent"))
        {
            for (const std::string& registrarName : componentRegistry->Names())
            {
                const std::unique_ptr<ecs::IComponentRegistrar> registrar = componentRegistry->Create(registrarName);
                registrar->Register(ecsWorld);
            }
        }
    }

    void StandaloneApplication::InitEcsWorld()
    {
        const std::filesystem::path worldPath = FindFileInParentTree("Suzanne.ecsbin");
        if (worldPath.empty())
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Warning,
                InitLabel,
                Message::Create("ECS world file not found: Suzanne.ecsbin")
            ));
            return;
        }

        const std::vector<uint8_t> worldBytes = LoadBinaryFile(worldPath);
        if (worldBytes.empty())
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Warning,
                InitLabel,
                Message::Create("ECS world file is empty: {}", worldPath.string())
            ));
            return;
        }

        MemoryDeserializer deserializer(worldBytes);
        if (!ecsWorld.DeserializeBinary(deserializer))
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Warning,
                InitLabel,
                Message::Create("Failed to deserialize ECS world '{}'", worldPath.string())
            ));
            return;
        }

        const std::vector<std::string> loadedPluginNames = pluginHost != nullptr
                                                               ? pluginHost->GetLoadedPluginNames()
                                                               : std::vector<std::string>{};
        for (const std::string& requiredPlugin : ecsWorld.GetRequiredPlugins())
        {
            const bool isLoaded = std::find(loadedPluginNames.begin(), loadedPluginNames.end(), requiredPlugin) != loadedPluginNames.end();
            if (!isLoaded)
            {
                compositeLogger->Log(CP_LOG_EVENT(
                    ILogger::Warning,
                    InitLabel,
                    Message::Create("ECS world requires missing plugin '{}'", requiredPlugin)
                ));
            }
        }

        if (Registry<ecs::ISystem>* ecsSystemRegistry =
            registryManager.Find<ecs::ISystem>("EcsSystem"))
        {
            for (const ecs::TypeGuid systemGuid : ecsWorld.GetStartupSystems())
            {
                const std::string systemKey = ecs::GuidToRegistryKey(systemGuid);
                if (!ecsSystemRegistry->Contains(systemKey))
                {
                    compositeLogger->Log(CP_LOG_EVENT(
                        ILogger::Warning,
                        InitLabel,
                        Message::Create("Missing ECS system for guid {}", systemKey)
                    ));
                    continue;
                }

                ecsSystems.push_back(ecsSystemRegistry->Create(systemKey));
            }
        }

        // const std::filesystem::path suzannePath = R"(D:\Projets\Cpp\Checkpoint\cmake-build-debug\Runtime\Standalone\suzanne.fbx)";
        // auto& meshManager = AssetRegistry::Instance().Get<Mesh>();
        // const AssetHandle<Mesh> suzanneMesh = meshManager.Load(suzannePath);
        //
        // if (!suzanneMesh.IsValid())
        // {
        //     compositeLogger->Log(CP_LOG_EVENT(
        //         ILogger::Warning,
        //         InitLabel,
        //         Message::Create("Failed to load mesh asset '{}'", suzannePath.string())
        //     ));
        // }
        //
        // for (int i = 0 ; i < 8 ; i++)
        // {
        //     const ecs::Entity suzanneEntity = ecsWorld.CreateEntity();
        //
        //     ecsWorld.AddComponent<Transform3D>(suzanneEntity, Transform3D{
        //         .x = -20.f + i * 5.f,
        //         .y = 0.0f,
        //         .z = -40.0f,
        //         .pitch = 0.0f,
        //         .yaw = (rand() / static_cast<float>(RAND_MAX)) * 180.f,
        //         .roll = 0.0f,
        //         .scaleX = 0.1f,
        //         .scaleY = 0.1f,
        //         .scaleZ = 0.1f
        //     });
        //
        //     ecsWorld.AddComponent<MeshRenderer>(suzanneEntity, MeshRenderer{
        //         .meshId = suzanneMesh,
        //         .visible = true
        //     });
        // }
        //
        // const ecs::Entity cameraEntity = ecsWorld.CreateEntity();
        // ecsWorld.AddComponent<Transform3D>(cameraEntity, Transform3D{
        //     .x = 0.0f,
        //     .y = 0.5f,
        //     .z = 0.0f,
        //     .pitch = 0.0f,
        //     .yaw = 0.0f,
        //     .roll = 0.0f,
        //     .scaleX = 1.0f,
        //     .scaleY = 1.0f,
        //     .scaleZ = 1.0f
        // });
        // ecsWorld.AddComponent<Camera>(cameraEntity, Camera{
        //     .fovYDegrees = 60.0f,
        //     .nearPlane = 0.1f,
        //     .farPlane = 1000.0f,
        //     .isPrimary = true,
        //     .enabled = true
        // });
        //
        // ecsWorld.SetStartupSystems({ ecs::MakeTypeGuid("ExamplePlugin.SpinSystem") });
        //
        // MemorySerializer serializer;
        // if (ecsWorld.SerializeBinary(serializer))
        // {
        //     const std::filesystem::path outputPath = std::filesystem::current_path() / "Suzanne.ecsbin";
        //     if (!SaveBinaryFile(outputPath, serializer.bytes))
        //     {
        //         compositeLogger->Log(CP_LOG_EVENT(
        //             ILogger::Warning,
        //             InitLabel,
        //             Message::Create("Failed to write ECS bootstrap scene '{}'", outputPath.string())
        //         ));
        //     }
        // }
        // else
        // {
        //     compositeLogger->Log(CP_LOG_EVENT(
        //         ILogger::Warning,
        //         InitLabel,
        //         Message::Create("Failed to serialize ECS bootstrap scene")
        //     ));
        // }
    }
}

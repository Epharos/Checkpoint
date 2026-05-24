#include <StandaloneApplication.hpp>

#include <Common/Async/JobSystem.hpp>
#include <Common/Core/Macros.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>
#include <Common/Util/Clock.hpp>

#include <Platform/Input/InputState.hpp>
#include <Platform/Input/RawInputState.hpp>

#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

#include <Resources/AssetRegistry.hpp>

#include <Rendering/BuiltinRenderPasses.hpp>
#include <Rendering/Renderer.hpp>
#include <Rendering/Scene/FrameGraphConfig.hpp>

#include <Runtime/Scene/SceneSerializer.hpp>

#include "PrecompiledShaderProvider.hpp"

#include <filesystem>
#include <string_view>
#include <thread>

#if defined(CP_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

namespace
{
    constexpr auto InitLabel = "Init";
    constexpr auto CleanupLabel = "Cleanup";
}

namespace cp
{
    StandaloneApplication::~StandaloneApplication() = default;

    bool StandaloneApplication::Init(int argc, char** argv)
    {
        std::string_view platformName = "GLFW";

        for (int i = 1; i < argc; ++i)
        {
            if (argv[i] == nullptr)
            {
                continue;
            }

            const std::string_view arg = argv[i];
            if (arg.starts_with("--platform="))
            {
                platformName = arg.substr(std::string_view("--platform=").size());
            }
            else if (!arg.starts_with("--") && scenePath.empty())
            {
                scenePath = std::filesystem::absolute(argv[i]);
            }
        }

        if (scenePath.empty())
        {
            scenePath = std::filesystem::current_path() / "Scene.scene";
        }

        const std::filesystem::path executableDir =
            (argc > 0 && argv[0] != nullptr)
                ? std::filesystem::absolute(argv[0]).parent_path()
                : std::filesystem::current_path();

        InitLoggers();
        InitRegistry();
        InitPlugins(executableDir);
        InitPlatform(executableDir, platformName);
        scene = std::make_unique<runtime::Scene>();
        RegisterPluginComponents();
        InitJobSystem();
        InitRenderingHardwareInterface();
        InitAssetRegistry();
        InitScene();
        InitWindow();
        InitRenderer();

        if (window != nullptr && renderer != nullptr)
        {
            window->SetResizeCallback([this](Extent2D<int> _newExtent)
            {
                renderer->Resize(_newExtent);
            });
        }

        if (scene != nullptr)
        {
            scene->GetWorld().SetResource(InputState{});
        }

        initialized = true;
        return true;
    }

    void StandaloneApplication::Run()
    {
        if (!initialized || window == nullptr || renderer == nullptr || scene == nullptr)
        {
            return;
        }

        Clock deltaTimeClock;

        while (!window->ShouldClose())
        {
            window->PollEvents();

            RawInputState rawInput;
            window->FillRawInputState(rawInput);
            scene->GetWorld().GetResource<InputState>().Update(rawInput);

            const float deltaTime = static_cast<float>(deltaTimeClock.Restart());
            for (const std::unique_ptr<ecs::ISystem>& system : scene->GetActiveSystems())
            {
                if (system)
                    system->Run(scene->GetWorld(), ecsCommandBuffer, deltaTime);
            }

            ecsCommandBuffer.Playback(scene->GetWorld());

            renderer->BeginFrame();
            renderer->Render();
            renderer->EndFrame();
        }

        scene->ShutdownSystems();
    }

    void StandaloneApplication::Clear()
    {
        if (scene != nullptr)
        {
            scene->Clear();
            scene.reset();
        }

        if (renderer != nullptr)
        {
            renderer->ResetFrameGraph();
            renderer.reset();
        }

        if (window != nullptr)
        {
            if (platformDestroyWindow != nullptr)
            {
                platformDestroyWindow(window);
            }

            window = nullptr;
            platformCreateWindow = nullptr;
            platformDestroyWindow = nullptr;
        }

        if (platformLibHandle != nullptr)
        {
#if defined(CP_PLATFORM_WINDOWS)
            FreeLibrary(static_cast<HMODULE>(platformLibHandle));
#endif
            platformLibHandle = nullptr;

            if (compositeLogger != nullptr)
            {
                compositeLogger->Log(CP_LOG_EVENT(ILogger::Info, CleanupLabel, Message::Create("Platform unloaded")));
            }
        }

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
        auto& renderPassRegistry = registryManager.GetOrCreate<IRenderPass>(std::string(cp::RenderPassRegistryName));
        registryManager.GetOrCreate<ecs::ISystem>(std::string(cp::EcsSystemRegistryName));
        registryManager.GetOrCreate<ecs::IComponentRegistrar>(std::string(cp::EcsComponentRegistryName));

#if defined(CP_DEVELOPMENT_BUILD)
        rendering::RegisterBuiltinRenderPasses(renderPassRegistry);
#endif
    }

    void StandaloneApplication::InitPlugins(const std::filesystem::path& _executableDir)
    {
        PluginHostContext pluginHostContext
        {
            .loadProfile = PluginHostLoadProfile::RuntimeOnly,
            .runtimeContext = PluginRuntimeContext{
                .mainLogger = compositeLogger.get(),
                .registryManager = &registryManager,
                .assetRegistry = &AssetRegistry::Instance()
            },
            .editorContext = PluginEditorContext{
                .mainLogger = compositeLogger.get(),
                .registryManager = &registryManager,
                .assetRegistry = &AssetRegistry::Instance()
            }
        };

        pluginHost = std::make_unique<PluginHost>(pluginHostContext);

        const std::filesystem::path pluginsDirectory = _executableDir / "Plugins";

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

    void StandaloneApplication::InitPlatform(const std::filesystem::path& _executableDir, std::string_view _platformName)
    {
        const std::filesystem::path platformsDir = _executableDir / "Platforms";
        const std::filesystem::path dllPath = platformsDir / ("Platform_" + std::string(_platformName) + ".dll");

        if (!std::filesystem::exists(dllPath))
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Error,
                InitLabel,
                Message::Create("Platform DLL not found: {}", dllPath.string())
            ));

            return;
        }

#if defined(CP_PLATFORM_WINDOWS)
        platformLibHandle = LoadLibraryW(dllPath.wstring().c_str());
#endif

        if (platformLibHandle == nullptr)
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Error,
                InitLabel,
                Message::Create("Failed to load platform DLL: {}", dllPath.string())
            ));

            return;
        }

#if defined(CP_PLATFORM_WINDOWS)
        const auto getDescriptor = reinterpret_cast<PlatformEntryFn>(
            GetProcAddress(static_cast<HMODULE>(platformLibHandle), PlatformEntryPointName.data())
        );
#endif

        if (getDescriptor == nullptr)
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Error,
                InitLabel,
                Message::Create("Platform DLL missing entry point '{}': {}", PlatformEntryPointName, dllPath.string())
            ));

#if defined(CP_PLATFORM_WINDOWS)
            FreeLibrary(static_cast<HMODULE>(platformLibHandle));
#endif

            platformLibHandle = nullptr;
            return;
        }

        const PlatformDescriptor* descriptor = getDescriptor();
        if (descriptor == nullptr || descriptor->apiVersion != PlatformApiVersion)
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Error,
                InitLabel,
                Message::Create("Platform DLL API version mismatch: {}", dllPath.string())
            ));

#if defined(CP_PLATFORM_WINDOWS)
            FreeLibrary(static_cast<HMODULE>(platformLibHandle));
#endif

            platformLibHandle = nullptr;
            return;
        }

        platformCreateWindow = descriptor->createWindow;
        platformDestroyWindow = descriptor->destroyWindow;

        compositeLogger->Log(CP_LOG_EVENT(
            ILogger::Info,
            InitLabel,
            Message::Create("Platform '{}' loaded from {}", descriptor->name, dllPath.string())
        ));
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
        if (platformCreateWindow == nullptr)
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Error,
                InitLabel,
                Message::Create("Cannot create window: no platform loaded")
            ));

            return;
        }

        WindowInfo windowInfo
        {
            .title = "Checkpoint Runtime (Standalone)",
            .extent = Extent2D{ 800, 600 },
            .resizable = true
        };

        window = platformCreateWindow(windowInfo);
    }

    void StandaloneApplication::InitRenderer()
    {
        shaderProvider = std::make_unique<PrecompiledShaderProvider>();

        RendererInfo renderInfo
        {
            .frameCount = 3,
            .extent = Extent2D{ 800, 600 },
            .imageFormat = Format::R8G8B8A8_UNORM,
            .nativeWindowHandle = window->GetNativeWindowHandle(),
            .registryManager = &registryManager,
            .ecsWorld = scene != nullptr ? &scene->GetWorld() : nullptr,
            .shaderProvider = shaderProvider.get()
        };

        renderer = std::make_unique<Renderer>(renderInfo, *rhi);

        if (scene != nullptr)
        {
            renderer->SetPendingPassBlobs(scene->GetPassBlobs());
            rendering::ApplyFrameGraphConfigToRenderer(
                *renderer,
                scene->GetActivePassNames(),
                scene->GetPassExecutionOrders()
            );
        }
    }

    void StandaloneApplication::RegisterPluginComponents()
    {
        if (const Registry<ecs::IComponentRegistrar>* componentRegistry =
            registryManager.Find<ecs::IComponentRegistrar>(cp::EcsComponentRegistryName))
        {
            for (const std::string& registrarName : componentRegistry->Names())
            {
                const std::unique_ptr<ecs::IComponentRegistrar> registrar = componentRegistry->Create(registrarName);
                if (scene != nullptr)
                    registrar->Register(scene->GetWorld());
            }
        }
    }

    void StandaloneApplication::InitScene()
    {
        compositeLogger->Log(CP_LOG_EVENT(
            ILogger::Info,
            InitLabel,
            Message::Create("Loading scene from '{}'", scenePath.string())
        ));

        if (!runtime::SceneSerializer::LoadSceneFromFile(*scene, scenePath, compositeLogger.get()))
        {
            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Warning,
                InitLabel,
                Message::Create("Failed to load scene '{}'", scenePath.string())
            ));

            return;
        }

        if (const Registry<ecs::ISystem>* ecsSystemRegistry =
            registryManager.Find<ecs::ISystem>(cp::EcsSystemRegistryName))
        {
            const size_t systemCount = scene->InitializeSystems(*ecsSystemRegistry);

            for (const std::string& guid : scene->GetEnabledSystemGuids())
            {
                if (!ecsSystemRegistry->Contains(guid))
                {
                    compositeLogger->Log(CP_LOG_EVENT(
                        ILogger::Warning,
                        InitLabel,
                        Message::Create("Missing ECS system for guid '{}'", guid)
                    ));
                }
            }

            compositeLogger->Log(CP_LOG_EVENT(
                ILogger::Info,
                InitLabel,
                Message::Create("Scene '{}' loaded ({} system(s), {} pass(es))",
                    scene->GetName(), systemCount, scene->GetActivePassNames().size())
            ));
        }
    }
}

#include <RuntimeEditorApplication.hpp>

#include <Common/Core/Registry.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>
#include <Common/Util/Clock.hpp>

#include <ECS/ECS.hpp>

#include <Platform/Input/InputState.hpp>
#include <Platform/Input/RawInputState.hpp>

#include <RHI/Data.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

#include <Rendering/IShaderProvider.hpp>
#include <Rendering/Renderer.hpp>
#include <Rendering/Scene/FrameGraphConfig.hpp>

#include <Runtime/Scene/Scene.hpp>

#include <Platform/PlatformAPI.hpp>

#include <atomic>
#include <vector>

extern "C" const cp::PlatformDescriptor* CP_GetPlatformDescriptor();

namespace
{
    constexpr auto RuntimeLabel = "Runtime";
}

namespace cp::runtime
{
    RuntimeEditorApplication::RuntimeEditorApplication(
        RenderingHardwareInterface& _rhi,
        std::shared_ptr<ILogger> _logger,
        std::unique_ptr<Scene> _scene,
        RegistryManager& _registryManager,
        IShaderProvider* _shaderProvider
    )
        : rhi(_rhi)
        , logger(std::move(_logger))
        , scene(std::move(_scene))
        , registryManager(_registryManager)
        , shaderProvider(_shaderProvider)
    {
    }

    RuntimeEditorApplication::~RuntimeEditorApplication()
    {
        Stop();
    }

    void RuntimeEditorApplication::SetOnStoppedCallback(std::function<void()> _callback)
    {
        onStoppedCallback = std::move(_callback);
    }

    void RuntimeEditorApplication::Start()
    {
        if (running.load())
        {
            return;
        }

        stopRequested.store(false);
        running.store(true);
        runtimeThread = std::thread(&RuntimeEditorApplication::RunLoop, this);
    }

    void RuntimeEditorApplication::Stop()
    {
        stopRequested.store(true);
        if (runtimeThread.joinable())
        {
            runtimeThread.join();
        }
    }

    bool RuntimeEditorApplication::IsRunning() const
    {
        return running.load();
    }

    void RuntimeEditorApplication::RunLoop()
    {
        const PlatformDescriptor* platformDesc = CP_GetPlatformDescriptor();
        if (platformDesc == nullptr || platformDesc->createWindow == nullptr)
        {
            if (logger)
            {
                logger->Log(CP_LOG_EVENT(ILogger::Error, RuntimeLabel,
                    Message::Create("Native platform descriptor is unavailable")));
            }

            running.store(false);
            if (onStoppedCallback) onStoppedCallback();
            return;
        }

        std::unique_ptr<IWindow, PlatformDestroyWindowFn> window(
            platformDesc->createWindow(WindowInfo{
                .title = "Checkpoint Runtime",
                .extent = Extent2D<int>{ 1280, 720 }
            }),
            platformDesc->destroyWindow
        );

        if (window == nullptr || window->GetNativeWindowHandle() == nullptr)
        {
            if (logger)
            {
                logger->Log(CP_LOG_EVENT(ILogger::Error, RuntimeLabel,
                    Message::Create("Failed to create runtime window")));
            }
            running.store(false);
            if (onStoppedCallback) onStoppedCallback();
            return;
        }

        cp::RendererInfo rendererInfo;
        rendererInfo.frameCount = 3;
        rendererInfo.extent = window->GetExtent();
        rendererInfo.imageFormat = cp::Format::R8G8B8A8_UNORM;
        rendererInfo.nativeWindowHandle = window->GetNativeWindowHandle();
        rendererInfo.registryManager = &registryManager;
        rendererInfo.ecsWorld = &scene->GetWorld();
        rendererInfo.shaderProvider = shaderProvider;

        auto renderer = std::make_unique<cp::Renderer>(rendererInfo, rhi);
        renderer->SetPendingPassBlobs(scene->GetPassBlobs());
        cp::rendering::ApplyFrameGraphConfigToRenderer(
            *renderer,
            scene->GetActivePassNames(),
            scene->GetPassExecutionOrders()
        );

        window->SetResizeCallback([&renderer](cp::Extent2D<int> _newExtent)
        {
            renderer->Resize(_newExtent);
        });

        size_t systemCount = 0;
        if (const Registry<ecs::ISystem>* sysReg = registryManager.Find<ecs::ISystem>(EcsSystemRegistryName))
        {
            systemCount = scene->InitializeSystems(*sysReg);

            for (const std::string& guid : scene->GetEnabledSystemGuids())
            {
                if (!sysReg->Contains(guid) && logger)
                {
                    logger->Log(CP_LOG_EVENT(ILogger::Warning, RuntimeLabel,
                        Message::Create("Missing ECS system for guid '{}'", guid)));
                }
            }
        }

        if (logger)
        {
            logger->Log(CP_LOG_EVENT(ILogger::Info, RuntimeLabel,
                Message::Create("Runtime started ({} system(s), {} pass(es))",
                    systemCount, scene->GetActivePassNames().size())));
        }

        scene->GetWorld().SetResource(InputState{});

        Clock deltaTimeClock;
        ecs::CommandBuffer commandBuffer;

        while (!stopRequested.load() && !window->ShouldClose())
        {
            window->PollEvents();

            RawInputState rawInput;
            window->FillRawInputState(rawInput);
            scene->GetWorld().GetResource<InputState>().Update(rawInput);

            const auto deltaTime = static_cast<float>(deltaTimeClock.Restart());

            for (const std::unique_ptr<ecs::ISystem>& system : scene->GetActiveSystems())
            {
                if (system)
                    system->Run(scene->GetWorld(), commandBuffer, deltaTime);
            }
            commandBuffer.Playback(scene->GetWorld());

            renderer->BeginFrame();
            renderer->Render();
            renderer->EndFrame();
        }

        scene->ShutdownSystems();
        renderer->ResetFrameGraph();
        renderer.reset();

        if (logger)
        {
            logger->Log(CP_LOG_EVENT(ILogger::Info, RuntimeLabel, Message::Create("Runtime stopped")));
        }

        running.store(false);
        if (onStoppedCallback) onStoppedCallback();
    }
}

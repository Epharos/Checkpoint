#include <RuntimeEditorApplication.hpp>

#include <Common/Core/Registry.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>
#include <Common/Util/Clock.hpp>

#include <ECS/ECS.hpp>

#include <RHI/Data.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

#include <Rendering/IShaderProvider.hpp>
#include <Rendering/Renderer.hpp>
#include <Rendering/Scene/FrameGraphConfig.hpp>

#include <Runtime/Scene/Scene.hpp>

#include <atomic>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <Windows.h>

namespace
{
    constexpr auto RuntimeLabel = "Runtime";
    constexpr wchar_t RuntimeWindowClassName[] = L"CheckpointRuntimeEditorWindow";
    constexpr int DefaultWindowWidth = 1280;
    constexpr int DefaultWindowHeight = 720;

    LRESULT CALLBACK RuntimeWindowProc(HWND _hwnd, UINT _msg, WPARAM _wp, LPARAM _lp)
    {
        if (_msg == WM_DESTROY)
        {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(_hwnd, _msg, _wp, _lp);
    }

    class RuntimeWindow
    {
    public:
        RuntimeWindow()
        {
            const HINSTANCE hInstance = GetModuleHandleW(nullptr);

            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = RuntimeWindowProc;
            wc.hInstance = hInstance;
            wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
            wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            wc.lpszClassName = RuntimeWindowClassName;

            classAtom = RegisterClassExW(&wc);
            if (classAtom == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            {
                return;
            }

            RECT rect = { 0, 0, DefaultWindowWidth, DefaultWindowHeight };
            AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

            hwnd = CreateWindowExW(
                0,
                RuntimeWindowClassName,
                L"Checkpoint Runtime",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                rect.right - rect.left,
                rect.bottom - rect.top,
                nullptr, nullptr, hInstance, nullptr
            );

            if (hwnd != nullptr)
            {
                ShowWindow(hwnd, SW_SHOW);
                UpdateWindow(hwnd);
            }
        }

        ~RuntimeWindow()
        {
            if (hwnd != nullptr)
            {
                DestroyWindow(hwnd);
                hwnd = nullptr;
            }
            if (classAtom != 0)
            {
                UnregisterClassW(RuntimeWindowClassName, GetModuleHandleW(nullptr));
                classAtom = 0;
            }
        }

        RuntimeWindow(const RuntimeWindow&) = delete;
        RuntimeWindow& operator=(const RuntimeWindow&) = delete;
        RuntimeWindow(RuntimeWindow&&) = delete;
        RuntimeWindow& operator=(RuntimeWindow&&) = delete;

        [[nodiscard]] bool IsValid() const { return hwnd != nullptr; }
        [[nodiscard]] void* GetNativeHandle() const { return hwnd; }

        [[nodiscard]] cp::Extent2D<int> GetExtent() const
        {
            RECT rect;
            if (GetClientRect(hwnd, &rect))
            {
                return cp::Extent2D<int>{
                    static_cast<int>(rect.right - rect.left),
                    static_cast<int>(rect.bottom - rect.top)
                };
            }

            return cp::Extent2D<int>{ DefaultWindowWidth, DefaultWindowHeight };
        }

        bool PollAndShouldContinue()
        {
            MSG msg;
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    return false;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            return hwnd != nullptr && IsWindow(hwnd);
        }

    private:
        HWND hwnd = nullptr;
        ATOM classAtom = 0;
    };
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
        RuntimeWindow window;

        if (!window.IsValid())
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

        const cp::Extent2D<int> extent = window.GetExtent();

        cp::RendererInfo rendererInfo;
        rendererInfo.frameCount = 3;
        rendererInfo.extent = extent;
        rendererInfo.imageFormat = cp::Format::R8G8B8A8_UNORM;
        rendererInfo.nativeWindowHandle = window.GetNativeHandle();
        rendererInfo.registryManager = &registryManager;
        rendererInfo.ecsWorld = &scene->GetWorld();
        rendererInfo.shaderProvider = shaderProvider;

        auto renderer = std::make_unique<cp::Renderer>(rendererInfo, rhi);
        renderer->SetPendingPassBlobs(scene->GetPassBlobs());
        cp::rendering::ApplyFrameGraphConfigToRenderer(*renderer, scene->GetActivePassNames());

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

        Clock deltaTimeClock;
        ecs::CommandBuffer commandBuffer;

        while (!stopRequested.load() && window.PollAndShouldContinue())
        {
            const float deltaTime = static_cast<float>(deltaTimeClock.Restart());

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

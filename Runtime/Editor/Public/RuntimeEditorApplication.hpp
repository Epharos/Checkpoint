#pragma once

#include <Common/Core/Log.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace cp
{
    class RenderingHardwareInterface;
    class RegistryManager;
    class IShaderProvider;
}

namespace cp::runtime
{
    class Scene;

    class RuntimeEditorApplication final
    {
    public:
        RuntimeEditorApplication(
            RenderingHardwareInterface& _rhi,
            std::shared_ptr<ILogger> _logger,
            std::unique_ptr<Scene> _scene,
            RegistryManager& _registryManager,
            IShaderProvider* _shaderProvider = nullptr
        );
        ~RuntimeEditorApplication();

        RuntimeEditorApplication(const RuntimeEditorApplication&) = delete;
        RuntimeEditorApplication& operator=(const RuntimeEditorApplication&) = delete;
        RuntimeEditorApplication(RuntimeEditorApplication&&) = delete;
        RuntimeEditorApplication& operator=(RuntimeEditorApplication&&) = delete;

        void SetOnStoppedCallback(std::function<void()> _callback);

        void Start();
        void Stop();

        [[nodiscard]] bool IsRunning() const;

    private:
        void RunLoop();

        RenderingHardwareInterface& rhi;
        std::shared_ptr<ILogger> logger;
        std::unique_ptr<Scene> scene;
        RegistryManager& registryManager;
        IShaderProvider* shaderProvider = nullptr;

        std::atomic<bool> running{ false };
        std::atomic<bool> stopRequested{ false };
        std::thread runtimeThread;
        std::function<void()> onStoppedCallback;
    };
}

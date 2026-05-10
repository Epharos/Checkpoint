#pragma once

#include <Common/Core/Log.hpp>
#include <Common/Plugin/PluginHost.hpp>

#include <Rendering/Renderer.hpp>

#include <Runtime/Scene/Scene.hpp>

// Temporarily include VulkanRHI, GLFWWindow

#include <VulkanRHI.hpp>
#include <GLFWWindow.hpp>

// ----

#include <ECS/ECS.hpp>

#include <filesystem>
#include <memory>

namespace cp
{
    class StandaloneApplication final
    {
    public:
        StandaloneApplication() = default;
        ~StandaloneApplication();

        StandaloneApplication(const StandaloneApplication&) = delete;
        StandaloneApplication& operator=(const StandaloneApplication&) = delete;
        StandaloneApplication(StandaloneApplication&&) = delete;
        StandaloneApplication& operator=(StandaloneApplication&&) = delete;

        bool Init(int argc, char** argv);
        void Run();
        void Clear();

    private:
        void InitLoggers();
        void InitRegistry();
        void InitPlugins(int argc, char** argv);
        void InitJobSystem();
        void InitRenderingHardwareInterface();
        void InitAssetRegistry();
        void InitWindow();
        void InitRenderer();

        void RegisterPluginComponents();
        void InitScene();

        std::shared_ptr<ILogger> compositeLogger;
        RegistryManager registryManager;

        std::unique_ptr<PluginHost> pluginHost;

        std::unique_ptr<runtime::Scene> scene;
        ecs::CommandBuffer ecsCommandBuffer;

        std::unique_ptr<RenderingHardwareInterface> rhi;
        std::unique_ptr<GLFWWindow> window;
        std::unique_ptr<Renderer> renderer;

        std::filesystem::path scenePath;

        bool initialized = false;
        bool jobSystemInitialized = false;
        bool assetRegistryInitialized = false;
    };
}

#pragma once

#include <Common/Core/Log.hpp>
#include <Common/Plugin/PluginHost.hpp>

#include <Rendering/IShaderProvider.hpp>
#include <Rendering/Renderer.hpp>

#include <Runtime/Scene/Scene.hpp>

#include <Platform/PlatformAPI.hpp>

// Temporarily include VulkanRHI
#include <VulkanRHI.hpp>

// ----

#include <ECS/ECS.hpp>

#include <filesystem>
#include <memory>
#include <string_view>

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
        void InitPlugins(const std::filesystem::path& _executableDir);
        void InitPlatform(const std::filesystem::path& _executableDir, std::string_view _platformName);
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

        IWindow* window = nullptr;
        void* platformLibHandle = nullptr;
        PlatformCreateWindowFn platformCreateWindow = nullptr;
        PlatformDestroyWindowFn platformDestroyWindow = nullptr;

        std::unique_ptr<IShaderProvider> shaderProvider;
        std::unique_ptr<Renderer> renderer;

        std::filesystem::path scenePath;

        bool initialized = false;
        bool jobSystemInitialized = false;
        bool assetRegistryInitialized = false;
    };
}

#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <Common/Data/Extent.hpp>

#include <RHI/Data.hpp>

#include "FrameGraph/FrameGraph.hpp"


namespace cp
{
    namespace ecs
    {
        class World;
    }

    enum class QueueType : uint8_t;
    enum class Format : uint32_t;
    class RenderingHardwareInterface;
    class ICommandAllocator;
    class ICommandBuffer;
    class ISwapchain;
    class ITimelineSemaphore;
    class RegistryManager;

    struct FrameContext
    {
        // 0 : Graphics, 1 : Compute, 2 : Copy
        std::array<std::unique_ptr<ICommandAllocator>, 3> commandAllocators;
        std::array<std::vector<std::unique_ptr<ICommandBuffer>>, 3> commandBuffers;

        uint64_t swapchainImageIndex;

        [[nodiscard]] ICommandAllocator& GetCommandAllocator(QueueType _queueType) const;
        [[nodiscard]] ICommandBuffer& GetCommandBuffer(QueueType _queueType, size_t _index = 0) const;

        explicit FrameContext(RenderingHardwareInterface& _rhi);
    };

    struct RendererInfo
    {
        uint32_t frameCount;
        Extent2D<int> extent;
        Format imageFormat;
        void* nativeWindowHandle;
        RegistryManager* registryManager = nullptr;
        ecs::World* ecsWorld = nullptr;
    };

    class Renderer
    {
    public:
        Renderer(RendererInfo& _info, RenderingHardwareInterface& _rhi);
        ~Renderer();

    public:
        void Resize(const Extent2D<int>& _newExtent);
        bool AddFrameGraphPass(std::string _passTypeName, bool _recompile = false);
        bool RemoveFrameGraphPass(std::string_view _passTypeName, bool _recompile = false);
        void RecompileFrameGraph();
        void ResetFrameGraph();

        /**
         * @brief Inject pass state blobs to be restored on the next framegraph rebuild.
         *
         * Typically called before RecompileFrameGraph() when loading a scene, so that
         * freshly-created pass instances receive their previously serialized state.
         *
         * @param _blobs Map of registry serialized bytes per type-name.
         */
        void SetPendingPassBlobs(std::unordered_map<std::string, std::vector<std::byte>> _blobs);

        /**
         * @brief Serialize the current state of all live pass instances.
         *
         * Useful just before saving a scene: the editor workspace calls this, stores the
         * result in the Scene description, then serializes the scene.
         *
         * @return Map of registry serialized bytes per type-name (only non-empty blobs).
         */
        [[nodiscard]] std::unordered_map<std::string, std::vector<std::byte>> SnapshotPassBlobs() const;

        /**
         * @brief Return a pointer to the live pass instance for the given registry type-name.
         * @return Pointer to the pass, or nullptr if not found / framegraph not compiled.
         */
        [[nodiscard]] IRenderPass* FindActivePass(std::string_view _passTypeName);

    public:
        void BeginFrame();
        void Render();
        void EndFrame();

    private:
        void Initialize();
        void Cleanup() const;

        void CreateFrameContext();
        void CreateSwapchain();
        void CreateInFlightFrameSemaphore();

        void RebuildFrameGraph();
        void BuildFrameGraph();
        void BlitFinalRenderingToSwapchain(const FrameContext& _context) const;

    protected:
        std::vector<FrameContext> frameContext;

        std::unique_ptr<ISwapchain> swapchain;
        std::unique_ptr<ITimelineSemaphore> inFlightFrameSemaphore;
        uint64_t* frameSignalValue = nullptr;
        uint64_t frameGlobalIndex = 0;

        RendererInfo rendererInfo;
        RenderingHardwareInterface& renderingHardwareInterface;

        uint32_t frameIndex = 0;

        std::vector<std::string> frameGraphPassTypeNames;

        std::unordered_map<std::string, std::vector<std::byte>> pendingPassBlobs;
        std::unordered_map<std::string, IRenderPass*> activePassByTypeName;

        FrameGraph frameGraph;
    };
}

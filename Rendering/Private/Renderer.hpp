#pragma once

#include <array>
#include <vector>
#include <memory>
#include <cstdint>

#include <Common/Data/Extent.hpp>

#include <RHI/Data.hpp>

#include <Resources/AssetHandle.hpp>

#include "FrameGraph/FrameGraph.hpp"


namespace cp
{
    enum class QueueType : uint8_t;
    enum class Format : uint32_t;
    class RenderingHardwareInterface;
    class ICommandAllocator;
    class ICommandBuffer;
    class ISwapchain;
    class ITimelineSemaphore;
    class IShaderModule;
    class IPipelineLayout;
    class IPipeline;

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
    };

    class Renderer
    {
    public:
        Renderer(RendererInfo& _info, RenderingHardwareInterface& _rhi);
        ~Renderer();

    public:
        void Resize(const Extent2D<int>& _newExtent);

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
        
        void BuildFrameGraph();
        void BlitFinalRenderingToSwapchain(const FrameContext& _context) const;

    protected:
        std::vector<FrameContext> frameContext;

        std::unique_ptr<ISwapchain> swapchain;
        std::unique_ptr<ITimelineSemaphore> inFlightFrameSemaphore;
        uint64_t* frameSignalValue = nullptr;
        uint64_t frameGlobalIndex = 0;

        RendererInfo& rendererInfo;
        RenderingHardwareInterface& renderingHardwareInterface;

        uint32_t frameIndex = 0;
        
        // FrameGraph
        FrameGraph frameGraph;

        // Rendering resources
        std::shared_ptr<ITexture> texture;
        std::shared_ptr<ITexture> depthTexture;

        std::shared_ptr<IShaderModule> logoShaderModule;
        std::shared_ptr<IPipelineLayout> logoPipelineLayout;
        std::shared_ptr<IPipeline> logoPipeline;

        std::shared_ptr<IDescriptorSetLayout> logoDescriptorSetLayout;
        std::shared_ptr<IDescriptorSet> logoDescriptorSet;

        AssetHandle<ITexture> logoTexture;
        std::shared_ptr<ISampler> logoSampler;

        std::shared_ptr<IShaderModule> negativeShaderModule;
        std::shared_ptr<IPipelineLayout> negativePipelineLayout;
        std::shared_ptr<IPipeline> negativePipeline;

        std::shared_ptr<IDescriptorSetLayout> negativeDescriptorSetLayout;
        std::shared_ptr<IDescriptorSet> negativeDescriptorSet;
    };
}

#pragma once

#include <array>
#include <vector>
#include <memory>
#include <cstdint>

#include <Common/Data/Extent.hpp>

#include <RHI/Data.hpp>


namespace cp
{
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
        void Resize(const Extent2D<int>& _newExtent) const;

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

    protected:
        std::vector<FrameContext> frameContext;

        std::unique_ptr<ISwapchain> swapchain;
        std::unique_ptr<ITimelineSemaphore> inFlightFrameSemaphore;
        uint64_t* frameSignalValue;
        uint64_t frameGlobalIndex = 0;

        RendererInfo& rendererInfo;
        RenderingHardwareInterface& renderingHardwareInterface;

        uint32_t frameIndex = 0;

        // tmp

        std::shared_ptr<cp::ITexture> texture;
        std::shared_ptr<cp::ITexture> depthTexture;
        std::shared_ptr<IShaderModule> triangleShaderModule;
        std::shared_ptr<IPipelineLayout> trianglePipelineLayout;
        std::shared_ptr<IPipeline> trianglePipeline;

    };
}

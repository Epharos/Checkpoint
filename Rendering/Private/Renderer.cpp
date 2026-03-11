#include "Renderer.hpp"

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/Synchro.hpp>

#include "../../RHI/Private/Synchro/IBarrier.hpp"

namespace cp
{
    FrameContext::FrameContext(RenderingHardwareInterface& _rhi) : swapchainImageIndex(0)
    {
        for (size_t i = 0; i < commandAllocators.size(); ++i)
        {
            commandAllocators[i] = _rhi.CreateCommandAllocator(
                _rhi.GetDevice().GetQueue(static_cast<QueueType>(i), 0)
            );

            // TODO : Add support for multi queue

            commandBuffers[i].emplace_back(std::move(commandAllocators[i]->Allocate()));
        }
    }

    Renderer::Renderer(RendererInfo& _info, RenderingHardwareInterface& _rhi)
        : rendererInfo(_info), renderingHardwareInterface(_rhi)
    {
        Initialize();
    }

    Renderer::~Renderer()
    {
        Cleanup();
    }

    void Renderer::Resize(const Extent2D<int>& _newExtent) const
    {
        CP_EXPECT_MSG(swapchain, "Cannot resize the Renderer without a created swapchain");

        rendererInfo.extent = _newExtent;
        swapchain->Resize(rendererInfo.extent);

        CP_ASSERT_MSG(
            rendererInfo.extent == swapchain->GetImageExtent(),
            "Swapchain extent does not match Renderer extent"
        );
    }

    void Renderer::BeginFrame()
    {
        FrameContext& context = frameContext[frameIndex];

        inFlightFrameSemaphore->WaitCPU(frameSignalValue[frameIndex]);

        for (size_t i = 0 ; i < context.commandAllocators.size() ; ++i)
        {
            context.commandAllocators[i]->Reset();
        }

        context.swapchainImageIndex = swapchain->AcquireNextImage();
    }

    void Renderer::Render()
    {
        FrameContext& context = frameContext[frameIndex];

        // tmp
        {
            ICommandBuffer& cmdBuffer = *context.commandBuffers[0][0];
            cmdBuffer.Begin();

            const TextureBarrierInfo toColorAttachment
            {
                .texture = *texture,
                .srcLayout = TextureLayout::Undefined,
                .dstLayout = TextureLayout::AttachmentOptimal,
                .srcStage = PipelineStage::Top,
                .dstStage = PipelineStage::ColorAttachment,
                .srcAccess = Access::None,
                .dstAccess = Access::ColorAttachmentWrite,
                .baseMip = 0,
                .mipCount = 1,
                .baseLayer = 0,
                .layerCount = 1
            };

            const TextureBarrierInfo toDepthAttachment
            {
                .texture = *depthTexture,
                .srcLayout = TextureLayout::Undefined,
                .dstLayout = TextureLayout::AttachmentOptimal,
                .srcStage = PipelineStage::Top,
                .dstStage = PipelineStage::EarlyDepth,
                .srcAccess = Access::None,
                .dstAccess = Access::DepthStencilWrite,
                .baseMip = 0,
                .mipCount = 1,
                .baseLayer = 0,
                .layerCount = 1
            };

            cmdBuffer.AddBarrier(IBarrier { toColorAttachment });
            cmdBuffer.AddBarrier(IBarrier { toDepthAttachment });

            cp::DepthStencilAttachmentInfo depthStencilAttachmentInfo
            {
                .texture = depthTexture.get(),
                .clearValue = cp::ClearDepthStencil {
                    .depth = 0.5f,
                    .stencil = 0
                },
            };

            cp::ColorAttachmentInfo colorAttachment
            {
                .texture = texture.get(),
                .clearValue = cp::Color(cp::ColorRGBA8(127))
            };

            cp::RenderingInfo renderingInfo
            {
                .extent = Extent2D{
                    static_cast<uint32_t>(rendererInfo.extent.x()),
                    static_cast<uint32_t>(rendererInfo.extent.y())
                },
                .layers = 1,
                .colorAttachments = { colorAttachment },
                .depthStencilAttachment = depthStencilAttachmentInfo
            };

            cmdBuffer.BeginRendering(renderingInfo);

            cmdBuffer.EndRendering();

            TextureBarrierInfo swapchainPresentLayoutBarrier
            {
                .texture = swapchain->GetSwapchainImage(context.swapchainImageIndex),
                .srcLayout = TextureLayout::Undefined,
                .dstLayout = TextureLayout::Present,
                .srcStage = PipelineStage::Top,
                .dstStage = PipelineStage::Bottom,
                .srcAccess = Access::None,
                .dstAccess = Access::None,
                .baseMip = 0,
                .mipCount = 1,
                .baseLayer = 0,
                .layerCount = 1
            };

            cmdBuffer.AddBarrier(IBarrier { swapchainPresentLayoutBarrier } );

            cmdBuffer.End();
        }
    }

    void Renderer::EndFrame()
    {
        FrameContext& context = frameContext[frameIndex];

        uint64_t signalValue = ++frameGlobalIndex;

        {
            cp::ICommandBuffer& cmdBuffer = *context.commandBuffers[0][0];

            SubmitInfo::SignalInfo inFlightFrameSignalInfo
            {
                .semaphore = inFlightFrameSemaphore.get(),
                .value = signalValue
            };

            SubmitInfo::SignalInfo renderFinishedSignalInfo
            {
                .semaphore = &swapchain->GetRenderFinishedSemaphore(),
                .value = signalValue
            };

            SubmitInfo::WaitInfo imageAvailableWaitInfo
            {
                .semaphore = &swapchain->GetImageAvailableSemaphore(),
                .value = swapchain->GetLastImageAvailableSignalValue()
            };

            const SubmitInfo submitInfo
            {
                .commandBuffers = { &cmdBuffer },
                .waitInfos = { imageAvailableWaitInfo },
                .signalInfos = { inFlightFrameSignalInfo, renderFinishedSignalInfo },
            };

            renderingHardwareInterface.GetDevice().GetQueue(QueueType::Graphics, 0).Submit(submitInfo);
        }

        frameSignalValue[frameIndex] = signalValue;

        swapchain->Present(frameGlobalIndex);

        frameIndex = (frameIndex + 1) % rendererInfo.frameCount;
    }

    void Renderer::CreateFrameContext()
    {
        frameContext.reserve(rendererInfo.frameCount);

        for (size_t i = 0; i < rendererInfo.frameCount; ++i)
        {
            frameContext.emplace_back(renderingHardwareInterface);
        }
    }

    void Renderer::CreateSwapchain()
    {
        const SwapchainInfo swapchainInfo
        {
            .extent = rendererInfo.extent,
            .imageCount = rendererInfo.frameCount,
            .format = rendererInfo.imageFormat,
            .nativeWindowHandle = rendererInfo.nativeWindowHandle
        };

        swapchain = renderingHardwareInterface.CreateSwapchain(swapchainInfo);
    }

    void Renderer::CreateInFlightFrameSemaphore()
    {
        inFlightFrameSemaphore = renderingHardwareInterface.CreateTimelineSemaphore();
    }

    void Renderer::Initialize()
    {
        CreateFrameContext();
        CreateSwapchain();
        CreateInFlightFrameSemaphore();

        frameSignalValue = new uint64_t[rendererInfo.frameCount];

        for (size_t i = 0; i < rendererInfo.frameCount; ++i)
        {
            frameSignalValue[i] = 0;
        }

        // TMP

        {
            const cp::TextureInfo textureInfo
            {
                .type = cp::TextureType::Texture2D,
                .extent = cp::Extent3D<uint32_t>{
                    static_cast<uint32_t>(rendererInfo.extent.x()),
                    static_cast<uint32_t>(rendererInfo.extent.y()),
                    1
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .format = cp::Format::R8G8B8A8_UNORM,
                .usage = cp::TextureUsage::ColorAttachment,
                .aspect = cp::TextureAspect::Color
            };

            const cp::TextureInfo depthTextureInfo
            {
                .type = cp::TextureType::Texture2D,
                .extent = cp::Extent3D<uint32_t>{
                    static_cast<uint32_t>(rendererInfo.extent.x()),
                    static_cast<uint32_t>(rendererInfo.extent.y()),
                    1
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .format = cp::Format::D24_UNORM_S8_UINT,
                .usage = cp::TextureUsage::DepthStencilAttachment,
                .aspect = cp::TextureAspect::DepthStencil
            };

            texture = renderingHardwareInterface.CreateTexture(textureInfo, TextureLayout::Undefined);
            depthTexture = renderingHardwareInterface.CreateTexture(depthTextureInfo, TextureLayout::Undefined);
        }
    }

    void Renderer::Cleanup() const
    {
        renderingHardwareInterface.GetDevice().WaitIdle();

        delete[] frameSignalValue;
    }
}

#include "Renderer.hpp"

#include <algorithm>
#include <string_view>
#include <vector>

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/Synchro.hpp>

#include <ECS/World.hpp>

#include <Common/Core/Registry.hpp>
#include <Common/Data/Rectangle.hpp>

#include "Resources/AssetRegistry.hpp"

namespace cp
{
    ICommandAllocator& FrameContext::GetCommandAllocator(QueueType _queueType) const
    {
        return *commandAllocators[static_cast<int>(_queueType)];
    }

    ICommandBuffer& FrameContext::GetCommandBuffer(QueueType _queueType, const size_t _index) const
    {
        CP_EXPECT_MSG(
            _index < commandBuffers[static_cast<int>(_queueType)].size(),
            "_index must be contained within the amount of allocated command buffers for this queue"
        );

        return *commandBuffers[static_cast<int>(_queueType)][_index];
    }

    FrameContext::FrameContext(RenderingHardwareInterface& _rhi) : swapchainImageIndex(0)
    {
        for (size_t i = 0; i < commandAllocators.size(); ++i)
        {
            commandAllocators[i] = _rhi.CreateCommandAllocator(
                _rhi.GetDevice().GetQueue(static_cast<QueueType>(i), 0)
            );

            // Create multiple command buffers for framegraph passes
            constexpr size_t maxCommandBuffers = 16;
            for (size_t j = 0; j < maxCommandBuffers; ++j)
            {
                commandBuffers[i].emplace_back(std::move(commandAllocators[i]->Allocate()));
            }
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

    void Renderer::Resize(const Extent2D<int>& _newExtent)
    {
        CP_EXPECT_MSG(swapchain, "Cannot resize the Renderer without a created swapchain");

        // Wait for GPU to complete all work
        renderingHardwareInterface.GetDevice().WaitIdle();

        swapchain->Resize(_newExtent);
        rendererInfo.extent = swapchain->GetImageExtent();

        RebuildFrameGraph();
    }

    bool Renderer::AddFrameGraphPass(std::string _passTypeName, const bool _recompile)
    {
        if (_passTypeName.empty())
        {
            return false;
        }

        if (
            std::find(
                frameGraphPassTypeNames.begin(),
                frameGraphPassTypeNames.end(),
                _passTypeName
            ) != frameGraphPassTypeNames.end()
        )
        {
            return false;
        }

        frameGraphPassTypeNames.push_back(std::move(_passTypeName));

        if (_recompile && frameGraph.IsCompiled())
        {
            RecompileFrameGraph();
        }

        return true;
    }

    bool Renderer::RemoveFrameGraphPass(const std::string_view _passTypeName, const bool _recompile)
    {
        if (_passTypeName.empty())
        {
            return false;
        }

        const auto it =
            std::find(frameGraphPassTypeNames.begin(), frameGraphPassTypeNames.end(), _passTypeName);

        if (it == frameGraphPassTypeNames.end())
        {
            return false;
        }

        frameGraphPassTypeNames.erase(it);

        if (_recompile && frameGraph.IsCompiled())
        {
            RecompileFrameGraph();
        }

        return true;
    }

    void Renderer::RecompileFrameGraph()
    {
        renderingHardwareInterface.GetDevice().WaitIdle();
        RebuildFrameGraph();
    }

    void Renderer::ResetFrameGraph()
    {
        renderingHardwareInterface.GetDevice().WaitIdle();
        frameGraph.Reset();
        frameGraphPassTypeNames.clear();
    }

    void Renderer::BeginFrame()
    {
        FrameContext& context = frameContext[frameIndex];

        inFlightFrameSemaphore->WaitCPU(frameSignalValue[frameIndex]);

        for (const auto & commandAllocator : context.commandAllocators)
        {
            commandAllocator->Reset();
        }

        context.swapchainImageIndex = swapchain->AcquireNextImage();
    }

    void Renderer::Render()
    {
        FrameContext& context = frameContext[frameIndex];
        frameGraph.Execute(context);
        BlitFinalRenderingToSwapchain(context);
    }

    void Renderer::BuildFrameGraph()
    {
        const Extent2D<uint32_t> renderExtent{
            static_cast<uint32_t>(rendererInfo.extent.x()),
            static_cast<uint32_t>(rendererInfo.extent.y())
        };

        if (rendererInfo.ecsWorld != nullptr)
        {
            std::shared_ptr<ecs::World> ecsWorldRef(rendererInfo.ecsWorld, [](ecs::World*) {});
            // Deleter should do nothing as the shared_ptr is not the actual owner of the ECS World
            frameGraph.ShareCpuResource<ecs::World>("ECS.World", std::move(ecsWorldRef));
        }

        CP_EXPECT_MSG(rendererInfo.registryManager != nullptr, "RendererInfo.registryManager must be provided");
        const Registry<IRenderPass>* passRegistry =
            rendererInfo.registryManager->Find<IRenderPass>("Renderpass");
        CP_EXPECT_MSG(passRegistry != nullptr, "Render pass registry 'Renderpass' is missing");

        const RenderPassInitContext initContext {
            .rhi = renderingHardwareInterface,
            .assetRegistry = AssetRegistry::Instance(),
            .renderExtent = renderExtent,
        };

        auto addConfiguredPass = [&](const std::string_view _passTypeName)
        {
            CP_EXPECT_MSG(passRegistry->Contains(_passTypeName), "Requested render pass type is not registered");
            std::unique_ptr<IRenderPass> pass = passRegistry->Create(_passTypeName);
            CP_ASSERT_MSG(pass != nullptr, "Registry returned a null render pass instance");

            auto* configurable = dynamic_cast<IConfigurableRenderPass*>(pass.get());
            CP_ASSERT_MSG(configurable != nullptr, "Registered render pass does not implement IConfigurableRenderPass");
            configurable->Configure(initContext);

            frameGraph.AddPass(std::move(pass));
        };

        for (const std::string& passTypeName : frameGraphPassTypeNames)
        {
            addConfiguredPass(passTypeName);
        }
    }

    void Renderer::BlitFinalRenderingToSwapchain(const FrameContext& _context) const
    {
        ITexture* finalRendering = frameGraph.GetFinalRendering();
        if (!finalRendering)
        {
            return;  // No final rendering produced
        }

        ITexture& swapchainImage = swapchain->GetSwapchainImage(_context.swapchainImageIndex);

        // Get a command buffer for the blit operation
        // Use index = frameGraph.GetPassCount() to get the next available buffer
        ICommandBuffer& cmd = _context.GetCommandBuffer(
            QueueType::Graphics,
            static_cast<uint32_t>(frameGraph.GetPassCount())
        );

        cmd.Begin();

        // Transition swapchain: Undefined -> TransferDst
        const TextureBarrierInfo swapchainToTransferDst{
            .texture = swapchainImage,
            .srcLayout = TextureLayout::Undefined,
            .dstLayout = TextureLayout::TransferDst,
            .srcStage = PipelineStage::Top,
            .dstStage = PipelineStage::Transfer,
            .srcAccess = Access::None,
            .dstAccess = Access::TransferWrite,
            .baseMip = 0,
            .mipCount = 1,
            .baseLayer = 0,
            .layerCount = 1
        };

        cmd.AddBarrier(IBarrier{ swapchainToTransferDst });

        // Get texture extents
        const auto& finalInfo = finalRendering->GetTextureInfo();
        const auto& swapchainInfo = swapchainImage.GetTextureInfo();

        // Setup blit region to copy entire texture
        const TextureBlitRegion blitRegion {
            .srcMipLevel = 0,
            .srcBaseArrayLayer = 0,
            .srcLayerCount = 1,
            .srcOffsets = {
                Offset3D{ 0, 0, 0 },
                Offset3D{ static_cast<int32_t>(finalInfo.extent.x()), static_cast<int32_t>(finalInfo.extent.y()), 1 }
            },
            .dstMipLevel = 0,
            .dstBaseArrayLayer = 0,
            .dstLayerCount = 1,
            .dstOffsets = {
                Offset3D{ 0, 0, 0 },
                Offset3D{ static_cast<int32_t>(swapchainInfo.extent.x()), static_cast<int32_t>(swapchainInfo.extent.y()), 1 }
            }
        };

        // Blit finalRendering to swapchain with linear filtering
        cmd.BlitTexture(*finalRendering, swapchainImage, blitRegion, Filter::Linear);

        // Transition swapchain: TransferDst -> Present
        const TextureBarrierInfo swapchainToPresent{
            .texture = swapchainImage,
            .srcLayout = TextureLayout::TransferDst,
            .dstLayout = TextureLayout::Present,
            .srcStage = PipelineStage::Transfer,
            .dstStage = PipelineStage::Bottom,
            .srcAccess = Access::TransferWrite,
            .dstAccess = Access::None,
            .baseMip = 0,
            .mipCount = 1,
            .baseLayer = 0,
            .layerCount = 1
        };

        cmd.AddBarrier(IBarrier{ swapchainToPresent });

        cmd.End();
    }

    void Renderer::EndFrame()
    {
        const FrameContext& context = frameContext[frameIndex];

        const uint64_t signalValue = ++frameGlobalIndex;

        // Collect all command buffers to submit
        std::vector<ICommandBuffer*> commandBuffersToSubmit;

        // Add framegraph command buffers
        const size_t passCount = frameGraph.GetPassCount();
        for (size_t i = 0; i < passCount; ++i)
        {
            commandBuffersToSubmit.push_back(&context.GetCommandBuffer(QueueType::Graphics, static_cast<uint32_t>(i)));
        }

        // Add blit command buffer
        commandBuffersToSubmit.push_back(&context.GetCommandBuffer(QueueType::Graphics, static_cast<uint32_t>(passCount)));

        // Setup submit info
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
            .commandBuffers = commandBuffersToSubmit,
            .waitInfos = { imageAvailableWaitInfo },
            .signalInfos = { inFlightFrameSignalInfo, renderFinishedSignalInfo },
        };

        renderingHardwareInterface.GetDevice().GetQueue(QueueType::Graphics, 0).Submit(submitInfo);

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
    }

    void Renderer::RebuildFrameGraph()
    {
        if (frameGraphPassTypeNames.empty())
        {
            return;
        }

        frameGraph.Reset();
        BuildFrameGraph();
        frameGraph.Compile(renderingHardwareInterface);
    }

    void Renderer::Cleanup() const
    {
        renderingHardwareInterface.GetDevice().WaitIdle();

        delete[] frameSignalValue;
    }
}

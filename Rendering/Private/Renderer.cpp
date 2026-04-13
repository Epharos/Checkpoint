#include "Renderer.hpp"

#include <filesystem>
#include <string_view>
#include <vector>

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/Synchro.hpp>

#include <Common/Data/Rectangle.hpp>
#include <Common/IO/FileHelper.hpp>

#include <Resources/AssetRegistry.hpp>

#include "FrameGraph/NegativePFX.hpp"
#include "FrameGraph/SceneRenderPass.hpp"

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

        rendererInfo.extent = _newExtent;
        swapchain->Resize(rendererInfo.extent);

        CP_ASSERT_MSG(
            rendererInfo.extent == swapchain->GetImageExtent(),
            "Swapchain extent does not match Renderer extent"
        );

        // Re-compile the framegraph with new dimensions
        // ClearResources keeps the passes, only re-creates resources
        frameGraph.ClearResources();
        frameGraph.Compile(renderingHardwareInterface);
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

        // Execute the framegraph
        frameGraph.Execute(context);

        // Blit the final rendering to the swapchain
        BlitFinalRenderingToSwapchain(context);
    }

    void Renderer::BuildFrameGraph()
    {
        const Extent2D<uint32_t> renderExtent{
            static_cast<uint32_t>(rendererInfo.extent.x()),
            static_cast<uint32_t>(rendererInfo.extent.y())
        };

        // Create the main scene rendering pass
        auto* scenePass = frameGraph.AddPassTyped(std::make_unique<SceneRenderPass>());
        auto* negativePFXPass = frameGraph.AddPassTyped(std::make_unique<NegativePostFX>());

        // Configure the pass with our rendering resources
        scenePass->GetData().pipeline = logoPipeline;
        scenePass->GetData().descriptorSet = logoDescriptorSet;
        scenePass->GetData().renderExtent = renderExtent;

        negativePFXPass->GetData().pipeline = negativePipeline;
        negativePFXPass->GetData().descriptorSet = negativeDescriptorSet;
        negativePFXPass->GetData().renderExtent = renderExtent;
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

            texture = renderingHardwareInterface.CreateTexture(textureInfo);
            depthTexture = renderingHardwareInterface.CreateTexture(depthTextureInfo);
        }

        const std::filesystem::path logoShaderPath = FindFileInParentTree("Logo.spv");
        CP_ASSERT_MSG(!logoShaderPath.empty(), "Could not find Logo.spv from current working directory hierarchy");

        const std::filesystem::path negativeShaderPath = FindFileInParentTree("NegativePFX.spv");
        CP_ASSERT_MSG(!negativeShaderPath.empty(), "Could not find NegativePFX.spv from current working directory hierarchy");

        const std::vector<uint8_t> logoShaderBytecode = LoadBinaryFile(logoShaderPath);
        const std::vector<uint8_t> negativeShaderBytecode = LoadBinaryFile(negativeShaderPath);

        const ShaderModuleInfo logoShaderModuleInfo
        {
            .stages = ShaderStage::Vertex | ShaderStage::Fragment,
            .bytecode = ShaderBytecode {
                .format = ShaderBinaryFormat::SpirV,
                .data = logoShaderBytecode.data(),
                .sizeBytes = logoShaderBytecode.size()
            }
        };

        const ShaderModuleInfo negativeShaderModuleInfo
        {
            .stages = ShaderStage::Compute,
            .bytecode = ShaderBytecode {
                .format = ShaderBinaryFormat::SpirV,
                .data = negativeShaderBytecode.data(),
                .sizeBytes = negativeShaderBytecode.size()
            }
        };

        logoShaderModule = renderingHardwareInterface.GetDevice().CreateShaderModule(logoShaderModuleInfo);
        negativeShaderModule = renderingHardwareInterface.GetDevice().CreateShaderModule(negativeShaderModuleInfo);

        const DescriptorSetLayoutInfo logoDescriptorSetLayoutInfo
        {
            .bindings = {
                DescriptorBinding {
                    .binding = 0,
                    .type = DescriptorType::CombinedImageSampler,
                    .count = 1,
                    .visibility = ShaderStage::Fragment
                }
            }
        };

        const DescriptorSetLayoutInfo negativeDescriptorSetLayoutInfo
        {
            .bindings = {
                DescriptorBinding {
                    .binding = 0,
                    .type = DescriptorType::StorageTexture,
                    .count = 1,
                    .visibility = ShaderStage::Compute
                }
            }
        };

        logoDescriptorSetLayout = renderingHardwareInterface.GetDevice().CreateDescriptorSetLayout(logoDescriptorSetLayoutInfo);
        negativeDescriptorSetLayout = renderingHardwareInterface.GetDevice().CreateDescriptorSetLayout(negativeDescriptorSetLayoutInfo);

        logoDescriptorSet = renderingHardwareInterface.GetDevice().CreateDescriptorSet(*logoDescriptorSetLayout);
        negativeDescriptorSet = renderingHardwareInterface.GetDevice().CreateDescriptorSet(*negativeDescriptorSetLayout);

        auto& textureManager = AssetRegistry::Instance().Get<ITexture>();
        logoTexture = textureManager.Load(FindFileInParentTree("logo.png"));
        logoSampler = renderingHardwareInterface.CreateSampler(SamplerInfo{});

        const DescriptorTextureBinding textureBinding {
            .binding = 0,
            .texture = logoTexture.Get(),
            .layout = TextureLayout::ShaderReadOnly,
            .sampler = logoSampler.get()
        };

        logoDescriptorSet->UpdateTextures({ textureBinding });

        const PipelineLayoutInfo logoPipelineLayoutInfo {
            .setLayouts = { logoDescriptorSetLayout }
        };

        const PipelineLayoutInfo negativePipelineLayoutInfo {
            .setLayouts = { negativeDescriptorSetLayout }
        };

        logoPipelineLayout = renderingHardwareInterface.GetDevice().CreatePipelineLayout(logoPipelineLayoutInfo);
        negativePipelineLayout = renderingHardwareInterface.GetDevice().CreatePipelineLayout(negativePipelineLayoutInfo);

        const GraphicsPipelineInfo graphicsPipelineInfo
        {
            .layout = logoPipelineLayout,
            .shaderModule = logoShaderModule,
            .stageMains = {
                { ShaderStage::Vertex, "vertexMain" },
                { ShaderStage::Fragment, "fragmentMain" },
            },
            .vertexInput = VertexInputState{},
            .topology = PrimitiveTopology::TriangleList,
            .rasterization = RasterizationState{},
            .depthStencil = DepthStencilState{},
            .blendAttachments = {},
            .colorAttachmentFormats = { Format::R8G8B8A8_UNORM },
            .depthStencilFormat = Format::D24_UNORM_S8_UINT
        };

        const ComputePipelineInfo negativeComputePipelineInfo
        {
            .layout = negativePipelineLayout,
            .computeShader = negativeShaderModule,
            .mainMethodName = "MainCS"
        };

        logoPipeline = renderingHardwareInterface.GetDevice().CreateGraphicsPipeline(graphicsPipelineInfo);
        negativePipeline = renderingHardwareInterface.GetDevice().CreateComputePipeline(negativeComputePipelineInfo);
        
        // Build and compile the framegraph
        BuildFrameGraph();
        frameGraph.Compile(renderingHardwareInterface);
    }

    void Renderer::Cleanup() const
    {
        renderingHardwareInterface.GetDevice().WaitIdle();

        delete[] frameSignalValue;
    }
}

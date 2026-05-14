#include "CommandBuffer.hpp"

#include <Common/Core/Assert.hpp>
#include <Common/Data/Viewport.hpp>
#include <Common/Data/Rectangle.hpp>

#include "CommandAllocator.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "../../../Private/Synchro/IBarrier.hpp"

#include "../Utilities/VulkanConverter.hpp"

#include "../Data/Texture.hpp"
#include "../Data/Buffer.hpp"
#include "../Data/DescriptorSet.hpp"
#include "../Rendering/Pipeline.hpp"
#include "../Rendering/PipelineLayout.hpp"

namespace cp
{
    namespace
    {
        void AddTextureBarrier(const TextureBarrierInfo& _barrierInfo, const vk::CommandBuffer& _commandBuffer)
        {
            vk::ImageMemoryBarrier2 barrierInfo;

            const auto& texture = static_cast<Texture&>(_barrierInfo.texture);

            barrierInfo.setImage(texture.GetImage());

            barrierInfo.setOldLayout(EnumCast<vk::ImageLayout>(_barrierInfo.srcLayout));
            barrierInfo.setNewLayout(EnumCast<vk::ImageLayout>(_barrierInfo.dstLayout));

            barrierInfo.setSrcAccessMask(EnumBitsCast<vk::AccessFlags2>(_barrierInfo.srcAccess));
            barrierInfo.setDstAccessMask(EnumBitsCast<vk::AccessFlags2>(_barrierInfo.dstAccess));

            barrierInfo.setSrcStageMask(EnumBitsCast<vk::PipelineStageFlags2>(_barrierInfo.srcStage));
            barrierInfo.setDstStageMask(EnumBitsCast<vk::PipelineStageFlags2>(_barrierInfo.dstStage));

            const vk::ImageSubresourceRange subresourceRange
            {
                EnumBitsCast<vk::ImageAspectFlags>(texture.GetTextureInfo().aspect),
                _barrierInfo.baseMip,
                _barrierInfo.mipCount,
                _barrierInfo.baseLayer,
                _barrierInfo.layerCount
            };

            barrierInfo.setSubresourceRange(subresourceRange);

            vk::DependencyInfo dependencyInfo;
            dependencyInfo.setImageMemoryBarrierCount(1);
            dependencyInfo.setPImageMemoryBarriers(&barrierInfo);

            _commandBuffer.pipelineBarrier2(dependencyInfo);

            _barrierInfo.texture.UpdateLayout(_barrierInfo.dstLayout);
        }

        void AddBufferBarrier(const BufferBarrierInfo& _barrierInfo, const vk::CommandBuffer& _commandBuffer)
        {
            vk::BufferMemoryBarrier2 barrierInfo;

            const auto& buffer = static_cast<Buffer&>(_barrierInfo.buffer);

            barrierInfo.setBuffer(buffer.GetHandle());
            barrierInfo.setOffset(_barrierInfo.offsetBytes);
            barrierInfo.setSize(_barrierInfo.sizeBytes);

            barrierInfo.setSrcAccessMask(EnumBitsCast<vk::AccessFlags2>(_barrierInfo.srcAccess));
            barrierInfo.setDstAccessMask(EnumBitsCast<vk::AccessFlags2>(_barrierInfo.dstAccess));

            barrierInfo.setSrcStageMask(EnumBitsCast<vk::PipelineStageFlags2>(_barrierInfo.srcStage));
            barrierInfo.setDstStageMask(EnumBitsCast<vk::PipelineStageFlags2>(_barrierInfo.dstStage));

            barrierInfo.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrierInfo.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

            vk::DependencyInfo dependencyInfo;
            dependencyInfo.setBufferMemoryBarrierCount(1);
            dependencyInfo.setPBufferMemoryBarriers(&barrierInfo);

            _commandBuffer.pipelineBarrier2(dependencyInfo);
        }
    }

    CommandBuffer::CommandBuffer(ICommandAllocator& _commandAllocator)
        : ICommandBuffer(_commandAllocator), commandAllocator(reinterpret_cast<CommandAllocator&>(_commandAllocator))
    {
        Initialize();
    }

    CommandBuffer::~CommandBuffer()
    {
        Cleanup();
    }

    void CommandBuffer::Begin()
    {
        vk::CommandBufferBeginInfo beginInfo;
        commandBuffer.begin(beginInfo);
    }

    void CommandBuffer::End()
    {
        commandBuffer.end();
    }

    void CommandBuffer::AddBarrier(const IBarrier& _barrier)
    {
        switch (_barrier.GetType())
        {
            case BarrierType::Texture:
                AddTextureBarrier(_barrier.GetTextureBarrierInfo(), commandBuffer);
                break;

            case BarrierType::Buffer:
                AddBufferBarrier(_barrier.GetBufferBarrierInfo(), commandBuffer);
                break;

            default:
                CP_ENSURE_MSG(false, "Unsupported barrier type");
                break;
        }
    }

    CommandBufferType CommandBuffer::GetType() const
    {
        switch (commandAllocator.GetQueue().GetType())
        {
            case QueueType::Graphics:
                return CommandBufferType::Graphics;

            case QueueType::Compute:
                return CommandBufferType::Compute;

            case QueueType::Transfer:
                return CommandBufferType::Copy;

            default:
                CP_ENSURE_MSG(false, "Unsupported queue type for command buffer");
                return CommandBufferType::Graphics;
        }
    }

    void CommandBuffer::BeginRendering(const RenderingInfo& _internalRenderingInfo)
    {
        vk::RenderingInfo renderingInfo;
        renderingInfo.setLayerCount(_internalRenderingInfo.layers);
        renderingInfo.setRenderArea(vk::Rect2D {
            vk::Offset2D {},
            vk::Extent2D { _internalRenderingInfo.extent.x(), _internalRenderingInfo.extent.y() }
        });
        renderingInfo.setColorAttachmentCount(static_cast<uint32_t>(_internalRenderingInfo.colorAttachments.size()));

        vk::RenderingAttachmentInfo depthAttachment;

        if (_internalRenderingInfo.depthStencilAttachment.has_value())
        {
            DepthStencilAttachmentInfo depthAttachmentInfo = *_internalRenderingInfo.depthStencilAttachment;

            vk::ClearValue clearValue;
            clearValue.depthStencil.depth = depthAttachmentInfo.clearValue.depth;
            clearValue.depthStencil.stencil = depthAttachmentInfo.clearValue.stencil;

            auto* texture = static_cast<Texture*>(depthAttachmentInfo.texture);
            CP_EXPECT_MSG(
                (texture->GetTextureInfo().usage & TextureUsage::DepthStencilAttachment) == TextureUsage::DepthStencilAttachment,
                "Depth attachment does not have a DepthStencilAttachment usage"
            );

            depthAttachment.setImageView(texture->GetImageView());
            depthAttachment.setImageLayout(vk::ImageLayout::eAttachmentOptimal);
            depthAttachment.setLoadOp(EnumCast<vk::AttachmentLoadOp>(depthAttachmentInfo.depthLoadOp));
            depthAttachment.setStoreOp(EnumCast<vk::AttachmentStoreOp>(depthAttachmentInfo.depthStoreOp));
            depthAttachment.setClearValue(clearValue);

            renderingInfo.setPDepthAttachment(&depthAttachment);
        }

        std::vector<vk::RenderingAttachmentInfo> renderingAttachments;

        for (const auto& attachment : _internalRenderingInfo.colorAttachments)
        {
            vk::RenderingAttachmentInfo attachmentDescription;

            vk::ClearValue clearValue;
            const ColorRGBA8 rgba8 = attachment.clearValue.ToRGBA8();
            clearValue.color.setFloat32({
                rgba8.Red() / 255.0f,
                rgba8.Green() / 255.0f,
                rgba8.Blue() / 255.0f,
                rgba8.Alpha() / 255.0f
            });

            auto* texture = static_cast<Texture*>(attachment.texture);

            CP_EXPECT_MSG(
                (texture->GetTextureInfo().usage & TextureUsage::ColorAttachment) == TextureUsage::ColorAttachment,
                "Color attachment does not have a ColorAttachment usage"
            );

            attachmentDescription.setImageView(texture->GetImageView());
            attachmentDescription.setImageLayout(vk::ImageLayout::eAttachmentOptimal);
            attachmentDescription.setLoadOp(EnumCast<vk::AttachmentLoadOp>(attachment.loadOp));
            attachmentDescription.setStoreOp(EnumCast<vk::AttachmentStoreOp>(attachment.storeOp));
            attachmentDescription.setClearValue(clearValue);

            renderingAttachments.push_back(attachmentDescription);
        }

        renderingInfo.setColorAttachmentCount(static_cast<uint32_t>(renderingAttachments.size()));
        renderingInfo.setPColorAttachments(renderingAttachments.data());



        commandBuffer.beginRendering(
            renderingInfo,
            commandAllocator.GetDevice().GetPhysicalDevice().GetInstance().GetDispatchLoaderDynamic()
        );
    }

    void CommandBuffer::EndRendering()
    {
        commandBuffer.endRendering();
    }

    void CommandBuffer::SetViewport(const Viewport& _viewport)
    {
        vk::Viewport viewport;
        viewport.setX(_viewport.GetX());
        viewport.setY(_viewport.GetY());
        viewport.setWidth(_viewport.GetWidth());
        viewport.setHeight(_viewport.GetHeight());
        viewport.setMinDepth(_viewport.GetMinDepth());
        viewport.setMaxDepth(_viewport.GetMaxDepth());

        commandBuffer.setViewport(0, 1, &viewport);
    }

    void CommandBuffer::SetScissor(const Rectangle2D& _rectangle)
    {
        vk::Rect2D scissor;
        scissor.setOffset(vk::Offset2D { _rectangle.offset.x(), _rectangle.offset.y() });
        scissor.setExtent(vk::Extent2D { _rectangle.extent.x(), _rectangle.extent.y() });

        commandBuffer.setScissor(0, 1, &scissor);
    }

    void CommandBuffer::BindPipeline(IPipeline& _pipeline)
    {
        auto& pipeline = static_cast<Pipeline&>(_pipeline);
        currentPipeline = &pipeline;
        
        vk::PipelineBindPoint bindPoint = _pipeline.GetType() == PipelineType::Graphics
            ? vk::PipelineBindPoint::eGraphics
            : vk::PipelineBindPoint::eCompute;

        commandBuffer.bindPipeline(bindPoint, pipeline.GetHandle());
    }

    void CommandBuffer::BindDescriptorSet(uint32_t _binding, IDescriptorSet& _set)
    {
        CP_ENSURE_MSG(currentPipeline != nullptr, "No pipeline is currently bound. Call BindPipeline before BindDescriptorSet.");

        auto& descriptorSet = static_cast<DescriptorSet&>(_set);
        
        auto& layout = static_cast<const PipelineLayout&>(
            currentPipeline->GetType() == PipelineType::Graphics 
                ? *std::get<const GraphicsPipelineInfo>(currentPipeline->GetInfo()).layout
                : *std::get<const ComputePipelineInfo>(currentPipeline->GetInfo()).layout
        );

        vk::PipelineBindPoint bindPoint = currentPipeline->GetType() == PipelineType::Graphics
            ? vk::PipelineBindPoint::eGraphics
            : vk::PipelineBindPoint::eCompute;

        commandBuffer.bindDescriptorSets(
            bindPoint,
            layout.GetHandle(),
            _binding,
            1,
            &descriptorSet.GetHandle(),
            0,
            nullptr
        );
    }

    void CommandBuffer::BindVertexBuffer(uint32_t _binding, IBuffer& _vertexBuffer)
    {
        auto& buffer = static_cast<Buffer&>(_vertexBuffer);
        vk::DeviceSize offset = 0;

        commandBuffer.bindVertexBuffers(_binding, 1, &buffer.GetHandle(), &offset);
    }

    void CommandBuffer::BindIndexBuffer(IBuffer& _indexBuffer, IndexType _indexType)
    {
        auto& buffer = static_cast<Buffer&>(_indexBuffer);
        vk::IndexType indexType = EnumCast<vk::IndexType>(_indexType);

        commandBuffer.bindIndexBuffer(buffer.GetHandle(), 0, indexType);
    }

    void CommandBuffer::Draw(
        uint32_t _vertexCount,
        uint32_t _instanceCount,
        uint32_t _firstVertex,
        uint32_t _firstInstance
    )
    {
        commandBuffer.draw(_vertexCount, _instanceCount, _firstVertex, _firstInstance);
    }

    void CommandBuffer::Dispatch(uint32_t _groupCountX, uint32_t _groupCountY, uint32_t _groupCountZ)
    {
        commandBuffer.dispatch(_groupCountX, _groupCountY, _groupCountZ);
    }

    void CommandBuffer::CopyBufferToTexture(
        IBuffer& _srcBuffer,
        ITexture& _dstTexture,
        const BufferTextureCopyRegion& _region)
    {
        auto& srcBuffer = static_cast<Buffer&>(_srcBuffer);
        auto& dstTexture = static_cast<Texture&>(_dstTexture);

        vk::BufferImageCopy copyRegion;
        copyRegion.setBufferOffset(_region.bufferOffset);
        copyRegion.setBufferRowLength(_region.bufferRowLength);
        copyRegion.setBufferImageHeight(_region.bufferImageHeight);

        vk::ImageSubresourceLayers subresource;
        subresource.setAspectMask(EnumBitsCast<vk::ImageAspectFlags>(dstTexture.GetTextureInfo().aspect));
        subresource.setMipLevel(_region.mipLevel);
        subresource.setBaseArrayLayer(_region.baseArrayLayer);
        subresource.setLayerCount(_region.layerCount);
        copyRegion.setImageSubresource(subresource);

        copyRegion.setImageOffset(vk::Offset3D(
            _region.textureOffset.x(),
            _region.textureOffset.y(),
            _region.textureOffset.z()
        ));

        copyRegion.setImageExtent(vk::Extent3D(
            _region.textureExtent.x(),
            _region.textureExtent.y(),
            _region.textureExtent.z()
        ));

        commandBuffer.copyBufferToImage(
            srcBuffer.GetHandle(),
            dstTexture.GetImage(),
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &copyRegion
        );
    }

    void CommandBuffer::CopyTexture(
        ITexture& _srcTexture,
        ITexture& _dstTexture,
        const TextureCopyRegion& _region)
    {
        auto& srcTexture = static_cast<Texture&>(_srcTexture);
        auto& dstTexture = static_cast<Texture&>(_dstTexture);

        vk::ImageCopy copyRegion;

        vk::ImageSubresourceLayers srcSubresource;
        srcSubresource.setAspectMask(EnumBitsCast<vk::ImageAspectFlags>(srcTexture.GetTextureInfo().aspect));
        srcSubresource.setMipLevel(_region.srcMipLevel);
        srcSubresource.setBaseArrayLayer(_region.srcBaseArrayLayer);
        srcSubresource.setLayerCount(_region.srcLayerCount);
        copyRegion.setSrcSubresource(srcSubresource);

        copyRegion.setSrcOffset(vk::Offset3D(
            _region.srcOffset.x(),
            _region.srcOffset.y(),
            _region.srcOffset.z()
        ));

        vk::ImageSubresourceLayers dstSubresource;
        dstSubresource.setAspectMask(EnumBitsCast<vk::ImageAspectFlags>(dstTexture.GetTextureInfo().aspect));
        dstSubresource.setMipLevel(_region.dstMipLevel);
        dstSubresource.setBaseArrayLayer(_region.dstBaseArrayLayer);
        dstSubresource.setLayerCount(_region.dstLayerCount);
        copyRegion.setDstSubresource(dstSubresource);

        copyRegion.setDstOffset(vk::Offset3D(
            _region.dstOffset.x(),
            _region.dstOffset.y(),
            _region.dstOffset.z()
        ));

        copyRegion.setExtent(vk::Extent3D(
            _region.extent.x(),
            _region.extent.y(),
            _region.extent.z()
        ));

        commandBuffer.copyImage(
            srcTexture.GetImage(),
            vk::ImageLayout::eTransferSrcOptimal,
            dstTexture.GetImage(),
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &copyRegion
        );
    }

    void CommandBuffer::BlitTexture(
        ITexture& _srcTexture,
        ITexture& _dstTexture,
        const TextureBlitRegion& _region,
        Filter _filter)
    {
        auto& srcTexture = static_cast<Texture&>(_srcTexture);
        auto& dstTexture = static_cast<Texture&>(_dstTexture);

        vk::ImageBlit blitRegion;

        vk::ImageSubresourceLayers srcSubresource;
        srcSubresource.setAspectMask(EnumBitsCast<vk::ImageAspectFlags>(srcTexture.GetTextureInfo().aspect));
        srcSubresource.setMipLevel(_region.srcMipLevel);
        srcSubresource.setBaseArrayLayer(_region.srcBaseArrayLayer);
        srcSubresource.setLayerCount(_region.srcLayerCount);
        blitRegion.setSrcSubresource(srcSubresource);

        std::array<vk::Offset3D, 2> srcOffsets = {
            vk::Offset3D(
                _region.srcOffsets[0].x(),
                _region.srcOffsets[0].y(),
                _region.srcOffsets[0].z()
            ),
            vk::Offset3D(
                _region.srcOffsets[1].x(),
                _region.srcOffsets[1].y(),
                _region.srcOffsets[1].z()
            )
        };
        blitRegion.setSrcOffsets(srcOffsets);

        vk::ImageSubresourceLayers dstSubresource;
        dstSubresource.setAspectMask(EnumBitsCast<vk::ImageAspectFlags>(dstTexture.GetTextureInfo().aspect));
        dstSubresource.setMipLevel(_region.dstMipLevel);
        dstSubresource.setBaseArrayLayer(_region.dstBaseArrayLayer);
        dstSubresource.setLayerCount(_region.dstLayerCount);
        blitRegion.setDstSubresource(dstSubresource);

        std::array<vk::Offset3D, 2> dstOffsets = {
            vk::Offset3D(
                _region.dstOffsets[0].x(),
                _region.dstOffsets[0].y(),
                _region.dstOffsets[0].z()
            ),
            vk::Offset3D(
                _region.dstOffsets[1].x(),
                _region.dstOffsets[1].y(),
                _region.dstOffsets[1].z()
            )
        };
        blitRegion.setDstOffsets(dstOffsets);

        commandBuffer.blitImage(
            srcTexture.GetImage(),
            vk::ImageLayout::eTransferSrcOptimal,
            dstTexture.GetImage(),
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &blitRegion,
            EnumCast<vk::Filter>(_filter)
        );
    }

    void CommandBuffer::Initialize()
    {
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.setCommandPool(commandAllocator.GetHandle());
        commandBufferAllocateInfo.setCommandBufferCount(1);
        commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

        commandBuffer = commandAllocator.GetDevice().GetHandle().allocateCommandBuffers(commandBufferAllocateInfo)[0];
    }

    void CommandBuffer::Cleanup() const
    {
        commandAllocator.GetDevice().GetHandle().freeCommandBuffers(commandAllocator.GetHandle(), commandBuffer);
    }
}

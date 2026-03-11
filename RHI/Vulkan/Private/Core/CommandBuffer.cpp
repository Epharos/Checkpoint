#include "CommandBuffer.hpp"

#include <Common/Core/Assert.hpp>

#include "CommandAllocator.hpp"
#include "Device.hpp"
#include "../../../Private/Synchro/IBarrier.hpp"

#include "../Utilities/VulkanConverter.hpp"

#include "../Data/Texture.hpp"

namespace cp
{
    namespace
    {
        void AddTextureBarrier(const TextureBarrierInfo& _barrierInfo, vk::CommandBuffer _commandBuffer)
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

            vk::ImageSubresourceRange subresourceRange
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
                // TODO : Add Buffer Barrier handle method
                break;
        }
    }

    CommandBufferType CommandBuffer::GetType() const
    {
        return CommandBufferType::Graphics; // TODO : Return the correct command buffer type
    }

    void CommandBuffer::BeginRendering(const RenderingInfo& _internalRenderingInfo)
    {
        vk::RenderingInfo renderingInfo;
        renderingInfo.setLayerCount(_internalRenderingInfo.layers);
        renderingInfo.setRenderArea({ vk::Offset2D {}, vk::Extent2D { _internalRenderingInfo.extent.x(), _internalRenderingInfo.extent.y() } });
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
                texture->GetTextureInfo().usage == TextureUsage::DepthStencilAttachment,
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
            clearValue.color.setUint32(attachment.clearValue.ToRGBA8().ToUInt32Array());

            auto* texture = static_cast<Texture*>(attachment.texture);

            CP_EXPECT_MSG(
                texture->GetTextureInfo().usage == TextureUsage::ColorAttachment,
                "Depth attachment does not have a ColorAttachment usage"
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

        commandBuffer.beginRendering(renderingInfo);
    }

    void CommandBuffer::EndRendering()
    {
        commandBuffer.endRendering();
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

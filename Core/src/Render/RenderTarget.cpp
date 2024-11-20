#include "RenderTarget.hpp"

#include "pch.hpp"

#include "../Helpers/MemoryHelper.hpp"

namespace Render
{
	RenderTarget::RenderTarget(const Context::VulkanContext& _context, const vk::Extent2D& _extent) : context(&_context), extent(_extent)
	{

	}

	RenderTarget::~RenderTarget()
	{
		for (auto& attachment : attachments)
		{
			attachment.Destroy(context->GetDevice());
		}

		context->GetDevice().destroyFramebuffer(framebuffer);
	}

	void RenderTarget::AddAttachment(const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags)
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D(extent.width, extent.height, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = _format;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = _usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		vk::Image image = context->GetDevice().createImage(imageInfo);

		AddAttachment(image, _format, _usage, _aspectFlags);
	}

	void RenderTarget::AddAttachment(const vk::Image& _image, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags)
	{
		vk::MemoryRequirements memRequirements = context->GetDevice().getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Helper::FindMemoryType(context->GetPhysicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::DeviceMemory imageMemory = context->GetDevice().allocateMemory(allocInfo);

		context->GetDevice().bindImageMemory(image, imageMemory, 0);

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = _image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = _format;
		viewInfo.subresourceRange.aspectMask = _aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vk::ImageView imageView = context->GetDevice().createImageView(viewInfo);

		attachments.push_back({ _image, imageView, imageMemory });
	}

	void RenderTarget::Build(const vk::RenderPass& _renderPass)
	{
		std::vector<vk::ImageView> attachmentViews;

		for (const auto& attachment : attachments)
		{
			attachmentViews.push_back(attachment.imageView);
		}

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.renderPass = _renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
		framebufferInfo.pAttachments = attachmentViews.data();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		framebuffer = context->GetDevice().createFramebuffer(framebufferInfo);
		renderPass = _renderPass;
	}
}

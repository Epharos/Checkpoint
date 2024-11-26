#include "pch.hpp"

#include "RenderTarget.hpp"

namespace Render
{
	RenderTarget::RenderTarget(Context::VulkanContext& _context, const vk::Extent2D& _extent) : context(&_context), extent(_extent)
	{

	}

	RenderTarget::~RenderTarget()
	{
		Cleanup();
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
		this->attachments.push_back(std::make_shared<RenderTargetAttachment>(context, _image, _format, _aspectFlags));
	}

	void RenderTarget::AddAttachment(std::shared_ptr<RenderTargetAttachment>& _attachment)
	{
		this->attachments.push_back(_attachment);
	}

	void RenderTarget::Build(const vk::RenderPass& _renderPass)
	{
		std::vector<vk::ImageView> attachmentViews;

		for (const auto& attachment : attachments)
		{
			attachmentViews.push_back(attachment->GetImageView());
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

	void RenderTarget::Cleanup()
	{
		context->GetDevice().destroyFramebuffer(framebuffer);
	}

	RenderTargetAttachment::RenderTargetAttachment(Context::VulkanContext* _context, const vk::Extent2D& _extent, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags)
	{
		context = _context;
		Build(_context, _extent, _format, _usage, _aspectFlags);
	}

	RenderTargetAttachment::RenderTargetAttachment(Context::VulkanContext* _context, const vk::Image& _image, const vk::Format& _format, const vk::ImageAspectFlags& _aspectFlags)
	{
		context = _context;
		Build(_context, _image, _format, _aspectFlags);
	}

	RenderTargetAttachment::~RenderTargetAttachment()
	{
		Destroy(context->GetDevice());
		LOG_TRACE("Destroyed RenderTargetAttachment");
	}

	void RenderTargetAttachment::Destroy(const vk::Device& device)
	{
		device.destroyImageView(imageView);
		if(!isSwapchain) device.destroyImage(image);
		device.freeMemory(imageMemory);
	}

	void RenderTargetAttachment::Build(Context::VulkanContext*& _context, const vk::Extent2D& _extent, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags)
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D(_extent.width, _extent.height, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = _format;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = _usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		image = _context->GetDevice().createImage(imageInfo);

		vk::MemoryRequirements memRequirements = _context->GetDevice().getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Helper::Memory::FindMemoryType(_context->GetPhysicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		imageMemory = _context->GetDevice().allocateMemory(allocInfo);

		_context->GetDevice().bindImageMemory(image, imageMemory, 0);

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = _format;
		viewInfo.subresourceRange.aspectMask = _aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		imageView = _context->GetDevice().createImageView(viewInfo);
	}
	
	void RenderTargetAttachment::Build(Context::VulkanContext*& _context, const vk::Image& _image, const vk::Format& _format, const vk::ImageAspectFlags& _aspectFlags)
	{
		image = _image;

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = _format;
		viewInfo.subresourceRange.aspectMask = _aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		imageView = _context->GetDevice().createImageView(viewInfo);
	}

	/*void RenderTargetAttachmentSwapchain::Destroy(const vk::Device& device)
	{
		device.destroyImageView(imageView);
		device.freeMemory(imageMemory);
	}

	RenderTargetAttachmentSwapchain::RenderTargetAttachmentSwapchain(Context::VulkanContext* _context, const vk::Extent2D& _extent, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags) : RenderTargetAttachment(_context, _extent, _format, _usage, _aspectFlags)
	{

	}

	RenderTargetAttachmentSwapchain::RenderTargetAttachmentSwapchain(Context::VulkanContext* _context, const vk::Image& _image, const vk::Format& _format, const vk::ImageAspectFlags& _aspectFlags) : RenderTargetAttachment(_context, _image, _format, _aspectFlags)
	{

	}

	RenderTargetAttachmentSwapchain::~RenderTargetAttachmentSwapchain()
	{
		Destroy(context->GetDevice());
		LOG_TRACE("Destroyed RenderTargetAttachmentSwapchain");
	}*/
}

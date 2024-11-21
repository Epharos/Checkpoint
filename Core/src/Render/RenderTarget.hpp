#pragma once

#include "../pch.hpp"

#include "../Context/VulkanContext.hpp"
#include "../Helpers/MemoryHelper.hpp"

namespace Render
{
	struct RenderTargetAttachment
	{
		vk::Image image;
		vk::ImageView imageView;
		vk::DeviceMemory imageMemory;

		void Destroy(const vk::Device& device)
		{
			device.destroyImageView(imageView);
			device.destroyImage(image);
			device.freeMemory(imageMemory);
		}

		void Build(const Context::VulkanContext*& _context, const vk::Extent2D& _extent, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags)
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
			allocInfo.memoryTypeIndex = Helper::FindMemoryType(_context->GetPhysicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

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
	};

	class RenderTarget
	{
	private:
		std::vector<RenderTargetAttachment*> attachments;

		vk::Framebuffer framebuffer;
		vk::RenderPass renderPass;

		vk::Extent2D extent;

		const Context::VulkanContext* context;

	public:
		RenderTarget(const Context::VulkanContext& _context, const vk::Extent2D& _extent);
		~RenderTarget();

		void AddAttachment(const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);
		void AddAttachment(const vk::Image& _image, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);
		void AddAttachment(RenderTargetAttachment*& _attachment);

		void Build(const vk::RenderPass& _renderPass);

		inline constexpr const std::vector<RenderTargetAttachment*>& GetAttachments() const { return attachments; }
		inline constexpr const RenderTargetAttachment* GetAttachment(const uint32_t& index) const { return attachments[index]; }
		inline constexpr const vk::Framebuffer& GetFramebuffer() const { return framebuffer; }
		inline constexpr const vk::Extent2D& GetExtent() const { return extent; }
	};
}
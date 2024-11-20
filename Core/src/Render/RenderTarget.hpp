#pragma once

#include "../pch.hpp"

#include "../Context/VulkanContext.hpp"

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
	};

	class RenderTarget
	{
	private:
		std::vector<RenderTargetAttachment> attachments;

		vk::Framebuffer framebuffer;
		vk::RenderPass renderPass;

		vk::Extent2D extent;

		const Context::VulkanContext* context;

	public:
		RenderTarget(const Context::VulkanContext& _context, const vk::Extent2D& _extent);
		~RenderTarget();

		void AddAttachment(const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);
		void AddAttachment(const vk::Image& _image, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);

		void Build(const vk::RenderPass& _renderPass);

		inline constexpr const std::vector<RenderTargetAttachment>& GetAttachments() const { return attachments; }
		inline constexpr const RenderTargetAttachment& GetAttachment(const uint32_t& index) const { return attachments[index]; }
		inline constexpr const vk::Framebuffer& GetFramebuffer() const { return framebuffer; }
		inline constexpr const vk::Extent2D& GetExtent() const { return extent; }
	};
}
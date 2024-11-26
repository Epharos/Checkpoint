#pragma once

#include "../../pch.hpp"

#include "../../Context/VulkanContext.hpp"
#include "../../Helpers/Helpers.hpp"

namespace Render
{
	class RenderTargetAttachment
	{
	protected:
		vk::Image image;
		vk::ImageView imageView;
		vk::DeviceMemory imageMemory;

		Context::VulkanContext* context;

		virtual void Destroy(const vk::Device& device);
		void Build(Context::VulkanContext*& _context, const vk::Extent2D& _extent, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);
		void Build(Context::VulkanContext*& _context, const vk::Image& _image, const vk::Format& _format, const vk::ImageAspectFlags& _aspectFlags);

	public:
		bool isSwapchain = false; //Temporary fix

		RenderTargetAttachment(Context::VulkanContext* _context, const vk::Extent2D& _extent, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);
		RenderTargetAttachment(Context::VulkanContext* _context, const vk::Image& _image, const vk::Format& _format, const vk::ImageAspectFlags& _aspectFlags);
		~RenderTargetAttachment();

		inline constexpr const vk::Image& GetImage() const { return image; }
		inline constexpr const vk::ImageView& GetImageView() const { return imageView; }
		inline constexpr const vk::DeviceMemory& GetImageMemory() const { return imageMemory; }
	};

	/*class RenderTargetAttachmentSwapchain : public RenderTargetAttachment
	{
	protected:
		virtual void Destroy(const vk::Device& device) override;

	public:
		RenderTargetAttachmentSwapchain(Context::VulkanContext* _context, const vk::Extent2D& _extent, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);
		RenderTargetAttachmentSwapchain(Context::VulkanContext* _context, const vk::Image& _image, const vk::Format& _format, const vk::ImageAspectFlags& _aspectFlags);
		~RenderTargetAttachmentSwapchain();
	};*/

	class RenderTarget
	{
	private:
		std::vector<std::shared_ptr<RenderTargetAttachment>> attachments;

		vk::Framebuffer framebuffer;
		vk::RenderPass renderPass;

		vk::Extent2D extent;

		Context::VulkanContext* context;

	public:
		RenderTarget(Context::VulkanContext& _context, const vk::Extent2D& _extent);
		~RenderTarget();

		void AddAttachment(const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);
		void AddAttachment(const vk::Image& _image, const vk::Format& _format, const vk::ImageUsageFlags _usage, const vk::ImageAspectFlags& _aspectFlags);
		void AddAttachment(std::shared_ptr<RenderTargetAttachment>& _attachment);

		void Build(const vk::RenderPass& _renderPass);

		void Cleanup();

		inline constexpr const vk::RenderPass& GetRenderPass() const { return renderPass; }
		inline constexpr const std::vector<std::shared_ptr<RenderTargetAttachment>>& GetAttachments() const { return attachments; }
		inline constexpr const std::shared_ptr<RenderTargetAttachment>& GetAttachment(const uint32_t& index) const { return attachments[index]; }
		inline constexpr const vk::Framebuffer& GetFramebuffer() const { return framebuffer; }
		inline constexpr const vk::Extent2D& GetExtent() const { return extent; }
	};
}
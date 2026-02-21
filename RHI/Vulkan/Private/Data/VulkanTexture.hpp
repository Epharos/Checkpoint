#pragma once

#include "../pch.hpp"

#include <ITexture.hpp>

namespace cp
{
	class VulkanDevice;
	class ILogger;

	struct VulkanTextureResource
	{
		vk::Image image{ VK_NULL_HANDLE };
		vk::DeviceMemory memory{ VK_NULL_HANDLE };

		bool isOwner = true; // Indicates whether this resource is owned by the VulkanTexture instance or if it was imported from elsewhere (e.g., swapchain images)

		std::shared_ptr<VulkanDevice> device = nullptr; // Pointer to the VulkanDevice that owns this resource, needed for cleanup if isOwner is true

		VulkanTextureResource() = default;
		/**
		* @brief Creates a VulkanTextureResource from an existing image. The image is not owned by the TextureResource.
		* 
		* @param _image The image that need to be stored by the TextureResource.
		*/
		VulkanTextureResource(vk::Image& _image);

		/**
		* @brief Creates a VulkanTextureResource created by the user via a VulkanTexture. The image is owned.
		* 
		* @param _info The information needed to create an image.
		* @param _device The VulkanDevice used to create the given image.
		*/
		VulkanTextureResource(const TextureInfo& _info, std::shared_ptr<VulkanDevice> _device);

		~VulkanTextureResource();
	};

	class VulkanTexture final : public ITexture
	{
	public:
		VulkanTexture(ILogger& _logger, vk::Image& _image);
		VulkanTexture(ILogger& _logger, const TextureInfo& _info, std::shared_ptr<VulkanDevice> _device);
		~VulkanTexture() override;

	private:
		void Initiate();
		void Cleanup();

	private:

		std::shared_ptr<VulkanTextureResource> resource; // Shared pointer to manage the lifetime of the Vulkan image and its memory, allowing for shared ownership in cases like swapchain images

		vk::ImageView view;

		std::shared_ptr<VulkanDevice> device;
	};
}
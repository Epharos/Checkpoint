#pragma once

#include "../pch.hpp"

#include <ITexture.hpp>

namespace cp
{
	class VulkanDevice;
	class ILogger;

	struct VulkanTextureResource
	{
	public:
		/**
		* @brief Creates a VulkanTextureResource from an existing image. The image is not owned by the TextureResource.
		* 
		* @param _image The image that need to be stored by the TextureResource.
		*/
		VulkanTextureResource(vk::Image& _image, VulkanDevice& _device);

		/**
		* @brief Creates a VulkanTextureResource created by the user via a VulkanTexture. The image is owned.
		* 
		* @param _info The information needed to create an image.
		* @param _device The VulkanDevice used to create the given image.
		*/
		VulkanTextureResource(const TextureInfo& _info, VulkanDevice& _device);

		~VulkanTextureResource();

		vk::Image& GetImage() { return image; }
		const vk::Image& GetImage() const { return image; }

		vk::DeviceMemory& GetMemory() { return memory; }
		const vk::DeviceMemory& GetMemory() const { return memory; }

		bool IsOwner() const { return isOwner; }

	private:
		vk::Image image{ VK_NULL_HANDLE };
		vk::DeviceMemory memory{ VK_NULL_HANDLE };

		bool isOwner = true;

		VulkanDevice& device;
	};

	class VulkanTexture final : public ITexture
	{
	public:
		/**
		* @brief Creates a VulkanTexture that wraps an existing image. The image is not owned by the VulkanTexture and will not be destroyed when the VulkanTexture is destroyed.
		* 
		* @param _logger The logger to log the creation of the VulkanTexture.
		* @param _image The image that need to be wrapped by the VulkanTexture.
		* @param _info The information about the given image.
		* @param _device The VulkanDevice used to create the given image.
		*/
		VulkanTexture(ILogger& _logger, vk::Image& _image, const TextureInfo& _info, VulkanDevice& _device);

		/**
		* @brief Creates a VulkanTexture that creates its own image. The image is owned by the VulkanTexture and will be destroyed when the VulkanTexture is destroyed.
		* 
		* @param _logger The logger to log the creation of the VulkanTexture.
		* @param _info The information needed to create an image.
		* @param _device The VulkanDevice used to create the given image.
		*/
		VulkanTexture(ILogger& _logger, const TextureInfo& _info, VulkanDevice& _device);
		~VulkanTexture() override;

		vk::ImageView& GetImageView() { return view; }
		const vk::ImageView& GetImageView() const { return view; }

	private:
		void Initiate();
		void Cleanup();

	private:
		std::unique_ptr<VulkanTextureResource> resource;
		vk::ImageView view { VK_NULL_HANDLE };

		VulkanDevice& device;
		ILogger& logger;
	};
}
#pragma once

#include "../pch.hpp"

#include <RHI/Data.hpp>

namespace cp
{
	class Device;
	class ILogger;

	struct TextureResource
	{
	public:
		/**
		* @brief Creates a VulkanTextureResource from an existing image. The image is not owned by the TextureResource.
		* 
		* @param _image The image that need to be stored by the TextureResource.
		* @param _device The Device used to create the given image.
		*/
		TextureResource(vk::Image& _image, Device& _device);

		/**
		* @brief Creates a VulkanTextureResource created by the user via a VulkanTexture. The image is owned.
		* 
		* @param _info The information needed to create an image.
		* @param _device The Device used to create the given image.
		*/
		TextureResource(const TextureInfo& _info, Device& _device, TextureLayout _initialLayout = TextureLayout::Undefined);

		~TextureResource();

		[[nodiscard]] vk::Image& GetImage() { return image; }
		[[nodiscard]] const vk::Image& GetImage() const { return image; }

		[[nodiscard]] vk::DeviceMemory& GetMemory() { return memory; }
		[[nodiscard]] const vk::DeviceMemory& GetMemory() const { return memory; }

		[[nodiscard]] bool IsOwner() const { return isOwner; }

	private:
		vk::Image image{ VK_NULL_HANDLE };
		vk::DeviceMemory memory{ VK_NULL_HANDLE };

		bool isOwner = true;

		Device& device;
	};

	class Texture final : public ITexture
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
		Texture(
			ILogger& _logger,
			vk::Image& _image,
			const TextureInfo& _info,
			Device& _device
		);

		/**
		* @brief Creates a VulkanTexture that creates its own image. The image is owned by the VulkanTexture and will be destroyed when the VulkanTexture is destroyed.
		* 
		* @param _logger The logger to log the creation of the VulkanTexture.
		* @param _info The information needed to create an image.
		* @param _device The VulkanDevice used to create the given image.
		*/
		Texture(
			ILogger& _logger,
			const TextureInfo& _info,
			Device& _device
		);

		~Texture() override;

		[[nodiscard]] vk::ImageView& GetImageView() { return view; }
		[[nodiscard]] const vk::ImageView& GetImageView() const { return view; }

		[[nodiscard]] vk::Image& GetImage() { return resource->GetImage(); }
		[[nodiscard]] const vk::Image& GetImage() const { return resource->GetImage(); }

		[[nodiscard]] vk::DeviceMemory& GetMemory() { return resource->GetMemory(); }
		[[nodiscard]] const vk::DeviceMemory& GetMemory() const { return resource->GetMemory(); }

	private:
		void Initiate();
		void Cleanup() const;

	private:
		std::unique_ptr<TextureResource> resource;
		vk::ImageView view { VK_NULL_HANDLE };

		Device& device;
		ILogger& logger;
	};
}
#include "../pch.hpp"

#include "Texture.hpp"

#include <Common/Core/Log.hpp>

#include "../Core/Device.hpp"
#include "../Core/PhysicalDevice.hpp"
#include "../Utilities/VulkanConverter.hpp"
#include "../Utilities/MemoryHelper.hpp"

#include <string_view>

namespace cp
{
	namespace
	{
		constexpr std::string_view to_string(const Format _format)
		{
			switch (_format)
			{
				case Format::R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
				case Format::B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
				case Format::R16G16B16A16_FLOAT: return "R16G16B16A16_FLOAT";
				case Format::R32_UINT: return "R32_UINT";

				case Format::D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
				case Format::D32_FLOAT: return "D32_FLOAT";
				default: return "Unknown";
			}
		}

		void CreateImage(vk::Image& _image, const TextureInfo& _info, Device& _device, TextureLayout _initialLayout = TextureLayout::Undefined)
		{
			vk::ImageCreateInfo imageCreateInfo = {};

			imageCreateInfo.setImageType(_info.extent.z() > 1 ? vk::ImageType::e3D : vk::ImageType::e2D);
			imageCreateInfo.setFormat(EnumCast<vk::Format, Format>(_info.format));
			imageCreateInfo.setExtent({ _info.extent.x(), _info.extent.y(), _info.extent.z() });
			imageCreateInfo.setMipLevels(_info.mipLevels);
			imageCreateInfo.setArrayLayers(_info.arrayLayers);
			imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
			imageCreateInfo.setUsage(EnumBitsCast<vk::ImageUsageFlags, TextureUsage>(_info.usage));
			imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageCreateInfo.setInitialLayout(EnumCast<vk::ImageLayout, TextureLayout>(_initialLayout));
			if (_info.type == TextureType::TextureCube)
				imageCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);

			CP_VK_CHECK(_device.GetHandle().createImage(&imageCreateInfo, nullptr, &_image));
			CP_ENSURE_MSG(_image != VK_NULL_HANDLE, "Image was not initialized");
		}

		void AllocateImageMemory(vk::DeviceMemory& _memory, vk::Image& _image, Device& _device)
		{
			vk::MemoryRequirements memoryRequirements = _device.GetHandle().getImageMemoryRequirements(_image);

			vk::MemoryAllocateInfo memoryAllocateInfo = {};
			memoryAllocateInfo.setAllocationSize(memoryRequirements.size);
			memoryAllocateInfo.setMemoryTypeIndex(
				FindMemoryType(
					_device.GetPhysicalDevice().GetHandle(),
					memoryRequirements.memoryTypeBits,
					vk::MemoryPropertyFlagBits::eDeviceLocal
				)
			);

			_memory = _device.GetHandle().allocateMemory(memoryAllocateInfo);

			CP_ENSURE_MSG(_memory != VK_NULL_HANDLE, "Memory was not initialized");
		}

		void BindImageMemory(vk::Image& _image, vk::DeviceMemory& _memory, Device& _device)
		{
			_device.GetHandle().bindImageMemory(_image, _memory, 0);
		}

		void CreateImageView(vk::ImageView& _view, const TextureInfo& _info, vk::Image& _image, Device& _device)
		{
			vk::ImageViewCreateInfo viewCreateInfo = {};
			viewCreateInfo.setImage(_image);
			viewCreateInfo.setFormat(EnumCast<vk::Format, Format>(_info.format));
			viewCreateInfo.setViewType(EnumCast<vk::ImageViewType, TextureType>(_info.type));

			vk::ImageSubresourceRange subresourceRange = {};
			subresourceRange.setBaseMipLevel(0);
			subresourceRange.setLevelCount(_info.mipLevels);
			subresourceRange.setBaseArrayLayer(0);
			subresourceRange.setLayerCount(_info.arrayLayers);
			subresourceRange.setAspectMask(EnumBitsCast<vk::ImageAspectFlags, TextureAspect>(_info.aspect));

			viewCreateInfo.setSubresourceRange(subresourceRange);

			CP_VK_CHECK(_device.GetHandle().createImageView(&viewCreateInfo, nullptr, &_view));
			CP_ENSURE_MSG(_view != VK_NULL_HANDLE, "View was not initialized");
		}
	}

	Texture::Texture(ILogger& _logger, const TextureInfo& _info, Device& _device)
		: ITexture(_info), device(_device), logger(_logger)
	{
		resource = std::make_unique<TextureResource>(_info, _device);

		Initiate();

		// _logger.Log(CP_LOG_EVENT(ILogger::Info, VulkanRHI_Label, cp::Message::Create<TextComponent>(
		// 	"Created texture (dim: {}x{}x{} format: {})",
		// 	info.extent.x(),
		// 	info.extent.y(),
		// 	info.extent.z(),
		// 	to_string(info.format))));

		CP_ENSURE_MSG(resource, "Could not create texture");
	}

	Texture::Texture(ILogger& _logger, vk::Image& _image, const TextureInfo& _info, Device& _device)
		: ITexture(_info), device(_device), logger(_logger)
	{
		resource = std::make_unique<TextureResource>(_image, _device);

		Initiate();

		// _logger.Log(CP_LOG_EVENT(ILogger::Info, VulkanRHI_Label, cp::Message::Create<TextComponent>("Texture access created")));

		CP_ENSURE_MSG(resource, "Could not store texture");
	}

	Texture::~Texture()
	{
		Cleanup();
	}

	void Texture::Initiate()
	{
		CreateImageView(view, info, resource->GetImage(), device);
	}

	void Texture::Cleanup() const
	{
		CP_EXPECT_MSG(view != VK_NULL_HANDLE, "View shouldn't be null");

		device.GetHandle().destroyImageView(view);
	}

	TextureResource::TextureResource(vk::Image& _image, Device& _device) :
		image(_image), memory(VK_NULL_HANDLE), device(_device), isOwner(false) {}

	TextureResource::TextureResource(const TextureInfo& _info, Device& _device, TextureLayout _initialLayout)
		: device(_device), isOwner(true)
	{
		CreateImage(image, _info, device, _initialLayout);
		AllocateImageMemory(memory, image, device);
		BindImageMemory(image, memory, device);
	}

	TextureResource::~TextureResource()
	{
		if (!isOwner) return;

		CP_EXPECT_MSG(image != VK_NULL_HANDLE, "Image shouldn't be null");
		CP_EXPECT_MSG(memory != VK_NULL_HANDLE, "Memory shouldn't be null");

		device.GetHandle().destroyImage(image);
		device.GetHandle().freeMemory(memory);
	}
}
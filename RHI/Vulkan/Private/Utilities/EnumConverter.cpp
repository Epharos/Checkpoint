#include "EnumConverter.hpp"

namespace cp
{
	vk::Format ConvertToVulkanFormat(Format _format)
	{
		switch (_format)
		{
			// Color formats
		case Format::R8G8B8A8_UNORM:
			return vk::Format::eR8G8B8A8Unorm;
		case Format::B8G8R8A8_UNORM:
			return vk::Format::eB8G8R8A8Unorm;
		case Format::R16G16B16A16_FLOAT:
			return vk::Format::eR16G16B16A16Sfloat;
		case Format::R32_UINT:
			return vk::Format::eR32Uint;

			// Depth formats
		case Format::D24_UNORM_S8_UINT:
			return vk::Format::eD24UnormS8Uint;
		case Format::D32_FLOAT:
			return vk::Format::eD32Sfloat;
		}

		throw std::logic_error("Unrecognized format!");
	}

	vk::ImageUsageFlagBits ConvertToVulkanImageUsageFlagBit(TextureUsage _usage)
	{
		switch (_usage)
		{
			case TextureUsage::Sampled:
				return vk::ImageUsageFlagBits::eSampled;
			case TextureUsage::ColorAttachment:
				return vk::ImageUsageFlagBits::eColorAttachment;
			case TextureUsage::DepthStencilAttachment:
				return vk::ImageUsageFlagBits::eDepthStencilAttachment;
			case TextureUsage::Storage:
				return vk::ImageUsageFlagBits::eStorage;
		}

		throw std::logic_error("Unrecognized texture usage!");
	}

	vk::ImageUsageFlags ConvertToVulkanImageUsageFlags(TextureUsage _usage)
	{
		return ConvertFromVulkan<TextureUsage, vk::ImageUsageFlags, ConvertToVulkanImageUsageFlagBit, 4>(_usage);
	}

	vk::ImageAspectFlagBits ConvertToVulkanImageAspectFlagBit(TextureAspect _aspect)
	{
		switch(_aspect)
		{
			case TextureAspect::Color:
				return vk::ImageAspectFlagBits::eColor;
			case TextureAspect::Depth:
				return vk::ImageAspectFlagBits::eDepth;
			case TextureAspect::Stencil:
				return vk::ImageAspectFlagBits::eStencil;
		}

		throw std::logic_error("Unrecognized texture aspect!");
	}

	vk::ImageAspectFlags ConvertToVulkanImageAspectFlags(TextureAspect _aspect)
	{
		return ConvertFromVulkan<TextureAspect, vk::ImageAspectFlags, ConvertToVulkanImageAspectFlagBit, 4>(_aspect);
	}

	vk::ImageViewType ConvertToVulkanImageViewType(TextureType _type)
	{
		switch (_type)
		{
			case TextureType::Texture2D:
				return vk::ImageViewType::e2D;
			case TextureType::Texture3D:
				return vk::ImageViewType::e3D;
			case TextureType::Texture2DArray:
				return vk::ImageViewType::e2DArray;
			default:
				throw std::logic_error("Unrecognized texture type!");
		}
	}
}
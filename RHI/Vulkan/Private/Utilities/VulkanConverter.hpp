#pragma once

#include "../pch.hpp"

#include <Converter.hpp>

#include <Formats.hpp>
#include <ITexture.hpp>

namespace cp
{
	template<>
	inline vk::Format EnumCast<vk::Format, Format>(Format _format)
	{
		switch (_format)
		{
			// Color formats
			case Format::R8G8B8A8_UNORM: return vk::Format::eR8G8B8A8Unorm;
			case Format::B8G8R8A8_UNORM: return vk::Format::eB8G8R8A8Unorm;
			case Format::R16G16B16A16_FLOAT: return vk::Format::eR16G16B16A16Sfloat;
			case Format::R32_UINT: return vk::Format::eR32Uint;

			// Depth formats
			case Format::D24_UNORM_S8_UINT: return vk::Format::eD24UnormS8Uint;
			case Format::D32_FLOAT: return vk::Format::eD32Sfloat;
		}

		throw std::logic_error("Unrecognized format!");
	}

	template<>
	inline cp::Format EnumCast<cp::Format, vk::Format>(vk::Format _format)
	{
		switch (_format)
		{
			case vk::Format::eR8G8B8A8Unorm: return cp::Format::R8G8B8A8_UNORM;
			case vk::Format::eB8G8R8A8Unorm: return cp::Format::B8G8R8A8_UNORM;
			case vk::Format::eR16G16B16A16Sfloat: return cp::Format::R16G16B16A16_FLOAT;
			case vk::Format::eR32Uint: return cp::Format::R32_UINT;

			case vk::Format::eD24UnormS8Uint: return cp::Format::D24_UNORM_S8_UINT;
			case vk::Format::eD32Sfloat: return cp::Format::D32_FLOAT;
		}

		throw std::logic_error("Unrecognized format!");
	}

	template<>
	inline vk::ImageUsageFlagBits EnumCast<vk::ImageUsageFlagBits, TextureUsage>(TextureUsage _usage)
	{
		switch (_usage)
		{
		case TextureUsage::Sampled: return vk::ImageUsageFlagBits::eSampled;
		case TextureUsage::ColorAttachment: return vk::ImageUsageFlagBits::eColorAttachment;
		case TextureUsage::DepthStencilAttachment: return vk::ImageUsageFlagBits::eDepthStencilAttachment;
		case TextureUsage::Storage: return vk::ImageUsageFlagBits::eStorage;
		}

		throw std::logic_error("Unrecognized texture usage!");
	}

	template<>
	inline vk::ImageUsageFlags EnumBitsCast<vk::ImageUsageFlags, TextureUsage>(TextureUsage _usage)
	{
		return ConvertBitField<vk::ImageUsageFlags, vk::ImageUsageFlagBits, TextureUsage, 4>(_usage);
	}

	template<>
	inline vk::ImageAspectFlagBits EnumCast<vk::ImageAspectFlagBits, TextureAspect>(TextureAspect _aspect)
	{
		switch (_aspect)
		{
		case TextureAspect::Color: return vk::ImageAspectFlagBits::eColor;
		case TextureAspect::Depth: return vk::ImageAspectFlagBits::eDepth;
		case TextureAspect::Stencil: return vk::ImageAspectFlagBits::eStencil;
		}

		throw std::logic_error("Unrecognized texture aspect!");
	}

	template<>
	inline vk::ImageAspectFlags EnumBitsCast<vk::ImageAspectFlags, TextureAspect>(TextureAspect _aspect)
	{
		return ConvertBitField<vk::ImageAspectFlags, vk::ImageAspectFlagBits, TextureAspect, 4>(_aspect);
	}

	template<>
	inline vk::ImageViewType EnumCast<vk::ImageViewType, TextureType>(TextureType _type)
	{
		switch (_type)
		{
		case TextureType::Texture2D: return vk::ImageViewType::e2D;
		case TextureType::Texture3D: return vk::ImageViewType::e3D;
		case TextureType::Texture2DArray: return vk::ImageViewType::e2DArray;
		}

		throw std::logic_error("Unrecognized texture type!");
	}
}
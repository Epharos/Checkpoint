#pragma once

#include "../pch.hpp"

#include <Formats.hpp>
#include <ITexture.hpp>

namespace cp
{
	template<Enum Src, typename Dst, auto Converter, size_t EnumSize>
	constexpr Dst ConvertFromVulkan(Src _src)
	{
		for (size_t i = 0; i < EnumSize; ++i)
		{
			if (static_cast<uint32_t>(_src) & (1 << i))
			{
				return Converter(static_cast<Src>(1 << i));
			}
		}

		throw std::logic_error("Unrecognized enum value!");
	}

	vk::Format ConvertToVulkanFormat(Format _format);

	vk::ImageUsageFlagBits ConvertToVulkanImageUsageFlagBit(TextureUsage _usage);
	vk::ImageUsageFlags ConvertToVulkanImageUsageFlags(TextureUsage _usage);

	vk::ImageAspectFlagBits ConvertToVulkanImageAspectFlagBit(TextureAspect _aspect);
	vk::ImageAspectFlags ConvertToVulkanImageAspectFlags(TextureAspect _aspect);

	vk::ImageViewType ConvertToVulkanImageViewType(TextureType _type);
}
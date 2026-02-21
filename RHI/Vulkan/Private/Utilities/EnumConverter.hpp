#pragma once

#include "../pch.hpp"

#include <Formats.hpp>
#include <ITexture.hpp>

namespace cp
{
	vk::Format ConvertToVulkanFormat(Format _format);
	vk::ImageUsageFlagBits ConvertToVulkanImageUsageFlagBit(TextureUsage _usage);
	vk::ImageUsageFlags ConvertToVulkanImageUsageFlags(TextureUsage _usage);
}
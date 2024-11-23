#pragma once

#include "../pch.hpp"

namespace Helper
{
	namespace Memory
	{
		uint32_t FindMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	}

	namespace Format
	{
		vk::Format FindSupportedFormat(const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format FindDepthFormat(const vk::PhysicalDevice& physicalDevice);
	}
}
#pragma once

#include "../pch.hpp"

namespace Context
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		constexpr bool IsComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& _device, const vk::SurfaceKHR& _surface);
}
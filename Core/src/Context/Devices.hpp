#pragma once

#include "../pch.hpp"

namespace cp
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;

		inline constexpr bool IsFullComplete() const
		{
			return IsGraphicsComplete() && IsComputeComplete() && IsPresentComplete();
		}

		inline constexpr bool IsGeneralComplete() const
		{
			return IsGraphicsComplete() && IsPresentComplete();
		}

		inline constexpr bool IsGraphicsComplete() const
		{
			return graphicsFamily.has_value();
		}

		inline constexpr bool IsComputeComplete() const
		{
			return computeFamily.has_value();
		}

		inline constexpr bool IsPresentComplete() const
		{
			return presentFamily.has_value();
		}
	};

	QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& _device);
}
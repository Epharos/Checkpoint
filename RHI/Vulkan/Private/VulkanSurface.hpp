#pragma once

#include "pch.hpp"

#include <ISurface.hpp>

namespace cp
{
	class VulkanInstance;

	class VulkanSurface final : public ISurface
	{
	public:
		VulkanSurface(SurfaceInfo _info, ILogger& _logger, VulkanInstance& _instance);
		virtual ~VulkanSurface();

	private:
		void Initialize();
		void Cleanup();

	private:
		vk::SurfaceKHR surface{ VK_NULL_HANDLE };

		VulkanInstance& instance;
	};
}
#pragma once

#include "pch.hpp"

#include <RHISurface.hpp>

namespace cp
{
	class VulkanInstance;

	class VulkanSurface final : public RHISurface
	{
	public:
		VulkanSurface(RHISurfaceInfo _info, ILogger& _logger, VulkanInstance& _instance);
		virtual ~VulkanSurface();

	private:
		void Initialize();
		void Cleanup();

	private:
		vk::SurfaceKHR surface{ VK_NULL_HANDLE };

		VulkanInstance& instance;
	};
}
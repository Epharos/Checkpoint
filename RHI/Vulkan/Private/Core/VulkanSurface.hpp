#pragma once

#include "../pch.hpp"

#include <ISurface.hpp>

namespace cp
{
	class VulkanInstance;

	class VulkanSurface final : public ISurface
	{
	public:
		VulkanSurface(ILogger & _logger, const SurfaceInfo& _info, VulkanInstance& _instance);
		virtual ~VulkanSurface();

	private:
		void Initialize();
		void Cleanup();

	private:
		vk::SurfaceKHR surface{ VK_NULL_HANDLE };

		VulkanInstance& instance;
	};
}
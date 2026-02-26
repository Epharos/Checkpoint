#pragma once

#include "../pch.hpp"

namespace cp
{
	// Forward declarations
	class VulkanInstance;
	class ILogger;

	struct SurfaceInfo
	{
		void* nativeWindowHandle = nullptr;
	};

	class VulkanSurface final
	{
	public:
		VulkanSurface(ILogger & _logger, const SurfaceInfo& _info, VulkanInstance& _instance);
		~VulkanSurface();

		vk::SurfaceKHR& GetHandle() { return surface; }
		const vk::SurfaceKHR& GetHandle() const { return surface; }

	private:
		void Initialize();
		void Cleanup();

	private:
		vk::SurfaceKHR surface{ VK_NULL_HANDLE };

		SurfaceInfo info;
		VulkanInstance& instance;

		ILogger& logger;
	};
}
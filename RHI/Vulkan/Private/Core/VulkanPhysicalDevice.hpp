#pragma once

#include "../pch.hpp"

#include "IPhysicalDevice.hpp"

#include <optional>

namespace cp
{
	class ILogger;
	class VulkanInstance;
	struct VulkanQueueFamilies;

	class VulkanPhysicalDevice final : public IPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(ILogger& _logger, VulkanInstance& _instance);
		~VulkanPhysicalDevice() override;
		
		void Initialize();
		void Cleanup();

		vk::PhysicalDevice& GetHandle() { return physicalDevice; }
		const vk::PhysicalDevice& GetHandle() const { return physicalDevice; }

		VulkanQueueFamilies FindQueueFamilies(std::optional<vk::SurfaceKHR> _surface = std::nullopt);

		std::unique_ptr<IDevice> CreateDevice() override;

	private:
		VulkanInstance& instance;

		vk::PhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	};
}
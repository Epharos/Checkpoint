#pragma once

#include "RHIPhysicalDevice.hpp"

#include <vulkan/vulkan.hpp>

namespace cp
{
	class ILogger;
	class VulkanInstance;
	struct VulkanQueueFamilies;

	class VulkanPhysicalDevice final : public RHIPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(ILogger& _logger, VulkanInstance& _instance);
		~VulkanPhysicalDevice() override;
		
		void Initialize();
		void Cleanup();

		vk::PhysicalDevice& GetHandle() { return physicalDevice; }
		const vk::PhysicalDevice& GetHandle() const { return physicalDevice; }

		VulkanQueueFamilies FindQueueFamilies();
		VulkanQueueFamilies FindQueueFamilies(vk::SurfaceKHR _surface);

		std::unique_ptr<RHIDevice> CreateDevice() override;

	private:
		VulkanInstance& instance;

		vk::PhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	};
}
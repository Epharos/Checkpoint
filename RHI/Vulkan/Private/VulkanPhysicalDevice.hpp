#pragma once

#include "RHIPhysicalDevice.hpp"

#include <vulkan/vulkan.hpp>

namespace cp
{
	class ILogger;
	class VulkanInstance;

	class VulkanPhysicalDevice final : public RHIPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(ILogger& _logger, VulkanInstance& _instance);
		~VulkanPhysicalDevice() override;
		
		void Initialize();
		void Cleanup();

		vk::PhysicalDevice& GetHandle() { return physicalDevice; }
		const vk::PhysicalDevice& GetHandle() const { return physicalDevice; }

	private:
		VulkanInstance& instance;

		vk::PhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	};
}
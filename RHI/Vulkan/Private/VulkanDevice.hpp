#pragma once

#include <RHIDevice.hpp>

#include <vulkan/vulkan.hpp>
#include <vector>
#include <array>

#include "VulkanQueue.hpp"

namespace cp
{
	class ILogger;
	class VulkanInstance;
	class VulkanPhysicalDevice;

	class VulkanDevice final : public RHIDevice
	{
	public:
		VulkanDevice(ILogger& _logger, VulkanPhysicalDevice& _physicalDevice);
		~VulkanDevice() override;

		RHIQueue& GetQueue(RHIQueueType _queueType, uint32_t _index) override;

	private:
		void Initialize();
		void Cleanup();

		bool CreateLogicalDevice();
		void CreateQueues();

	private:
		vk::Device device{ VK_NULL_HANDLE };
		VulkanQueueFamilies families;

		// 0 : Graphics, 1 : Compute, 2 : Transfer
		std::array<std::vector<std::unique_ptr<VulkanQueue>>, 3> queues; 

		VulkanPhysicalDevice& physicalDevice;
	};
}
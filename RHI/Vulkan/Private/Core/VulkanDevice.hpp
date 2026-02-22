#pragma once

#include "../pch.hpp"

#include <IDevice.hpp>

#include <vector>
#include <array>

#include "VulkanQueue.hpp"

namespace cp
{
	class ILogger;
	class VulkanInstance;
	class VulkanPhysicalDevice;

	class VulkanDevice final : public IDevice
	{
	public:
		VulkanDevice(ILogger& _logger, VulkanPhysicalDevice& _physicalDevice);
		~VulkanDevice() override;

		IQueue& GetQueue(IQueueType _queueType, uint32_t _index) override;
		std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info) override;

		vk::Device& GetHandle() { return device; }
		const vk::Device& GetHandle() const { return device; }

		const VulkanPhysicalDevice& GetPhysicalDevice() const { return physicalDevice; }

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
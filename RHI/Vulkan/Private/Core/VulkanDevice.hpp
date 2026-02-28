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
	public: // Constructors, destructor, operators
		VulkanDevice(ILogger& _logger, VulkanPhysicalDevice& _physicalDevice);
		~VulkanDevice() override;

	public: // Getters and Setters
		IQueue& GetQueue(QueueType _queueType, uint32_t _index) override;

		vk::Device& GetHandle() { return device; }
		const vk::Device& GetHandle() const { return device; }

		VulkanPhysicalDevice& GetPhysicalDevice() { return physicalDevice; }
		const VulkanPhysicalDevice& GetPhysicalDevice() const { return physicalDevice; }

		VulkanQueueFamilies& GetQueueFamilies() { return families; }
		const VulkanQueueFamilies& GetQueueFamilies() const { return families; }

	public: // Resource creation
		std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info) override;

	private: // Initialization and cleanup
		void Initialize();
		void Cleanup();

		bool CreateLogicalDevice();
		void CreateQueues();

	public: // Override methods
		void WaitForIdle() const override;

	private:
		vk::Device device{ VK_NULL_HANDLE };
		VulkanQueueFamilies families;

		// 0 : Graphics, 1 : Compute, 2 : Transfer
		std::array<std::vector<std::unique_ptr<VulkanQueue>>, 3> queues; 

		VulkanPhysicalDevice& physicalDevice;
	};
}
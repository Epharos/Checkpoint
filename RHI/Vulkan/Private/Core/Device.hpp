#pragma once

#include "../pch.hpp"

#include <RHI/Core.hpp>

#include <vector>
#include <array>

#include "Queue.hpp"

namespace cp
{
	class ILogger;
	class Instance;
	class PhysicalDevice;

	class Device final : public IDevice
	{
	public: // Constructors, destructor, operators
		Device(ILogger& _logger, PhysicalDevice& _physicalDevice);
		~Device() override;

	public: // Getters and Setters
		IQueue& GetQueue(QueueType _queueType, uint32_t _index) override;

		[[nodiscard]] vk::Device& GetHandle() { return device; }
		[[nodiscard]] const vk::Device& GetHandle() const { return device; }

		[[nodiscard]] PhysicalDevice& GetPhysicalDevice() { return physicalDevice; }
		[[nodiscard]] const PhysicalDevice& GetPhysicalDevice() const { return physicalDevice; }

		[[nodiscard]] VulkanQueueFamilies& GetQueueFamilies() { return families; }
		[[nodiscard]] const VulkanQueueFamilies& GetQueueFamilies() const { return families; }

	public: // Resource creation
		std::shared_ptr<ITexture> CreateTexture(const TextureInfo& _info) override;

	private: // Initialization and cleanup
		void Initialize();
		void Cleanup();

		bool CreateLogicalDevice();
		void CreateQueues();

	public: // Override methods
		void WaitIdle() const override;

	private:
		vk::Device device{ VK_NULL_HANDLE };
		VulkanQueueFamilies families;

		// 0 : Graphics, 1 : Compute, 2 : Transfer
		std::array<std::vector<std::unique_ptr<Queue>>, 3> queues; 

		PhysicalDevice& physicalDevice;
	};
}
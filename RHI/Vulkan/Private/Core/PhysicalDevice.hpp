#pragma once

#include "../pch.hpp"

#include "IPhysicalDevice.hpp"

#include <optional>

namespace cp
{
	class ILogger;
	class Instance;
	struct VulkanQueueFamilies;

	class PhysicalDevice final : public IPhysicalDevice
	{
	public:
		PhysicalDevice(ILogger& _logger, Instance& _instance);
		~PhysicalDevice() override;
		
		void Initialize();
		void Cleanup();

		vk::PhysicalDevice& GetHandle() { return physicalDevice; }
		const vk::PhysicalDevice& GetHandle() const { return physicalDevice; }

		VulkanQueueFamilies FindQueueFamilies(std::optional<vk::SurfaceKHR> _surface = std::nullopt);

		Instance& GetInstance() { return instance; }
		const Instance& GetInstance() const { return instance; }

	private:
		Instance& instance;

		vk::PhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	};
}
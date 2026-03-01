#include "../pch.hpp"

#include "PhysicalDevice.hpp"

#include <Log.hpp>
#include <Profiling.hpp>

#include "Instance.hpp"
#include "Queue.hpp"
#include "Device.hpp"

namespace cp
{
	namespace
	{
		size_t ComputeDeviceScore(const vk::PhysicalDevice& device)
		{
			size_t score = 0;
			auto properties = device.getProperties();
			
			if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				score += 1000;

			return score;
		}

		void StoreGraphicsQueueIndex(VulkanQueueFamilies& _queueFamilies, const vk::QueueFamilyProperties& _properties, uint32_t _index)
		{
			if (_properties.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				_queueFamilies.graphics = _index;
			}
		}

		void StoreComputeQueueIndex(VulkanQueueFamilies& _queueFamilies, const vk::QueueFamilyProperties& _properties, uint32_t _index)
		{
			if (_properties.queueFlags & vk::QueueFlagBits::eCompute &&
				!(_properties.queueFlags & vk::QueueFlagBits::eGraphics))
			{
				_queueFamilies.compute = _index;
			}
		}

		void StoreTransferQueueIndex(VulkanQueueFamilies& _queueFamilies, const vk::QueueFamilyProperties& _properties, uint32_t _index)
		{
			if (_properties.queueFlags & vk::QueueFlagBits::eTransfer &&
				!(_properties.queueFlags & vk::QueueFlagBits::eGraphics) &&
				!(_properties.queueFlags & vk::QueueFlagBits::eCompute))
			{
				_queueFamilies.transfer = _index;
			}
		}

		void StorePresentQueueIndex(VulkanQueueFamilies& _queueFamilies, const vk::PhysicalDevice& _device, vk::SurfaceKHR _surface, uint32_t _index)
		{
			if (_device.getSurfaceSupportKHR(_index, _surface))
			{
				_queueFamilies.present = _index;
			}
		}

		void QueueIndicesFallbacks(VulkanQueueFamilies& _queueFamilies)
		{
			if (_queueFamilies.compute == std::numeric_limits<uint32_t>::max())
			{
				_queueFamilies.compute = _queueFamilies.graphics;
			}

			if (_queueFamilies.transfer == std::numeric_limits<uint32_t>::max())
			{
				_queueFamilies.transfer = _queueFamilies.compute;
			}

			if (_queueFamilies.present == std::numeric_limits<uint32_t>::max())
			{
				_queueFamilies.present = _queueFamilies.graphics;
			}
		}
	}

	PhysicalDevice::PhysicalDevice(ILogger& _logger, Instance& _instance)
		: IPhysicalDevice(_logger), instance(_instance)
	{
		Initialize();
	}

	PhysicalDevice::~PhysicalDevice()
	{
		Cleanup();
	}

	void PhysicalDevice::Initialize()
	{
		CP_PROFILE_SCOPE("VulkanPhysicalDevice#Initialize");

		auto devices = instance.GetHandle().enumeratePhysicalDevices();

		if (devices.empty())
		{
			logger.Log(CP_LOG_EVENT(cp::ILogger::Critical, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Failed to find any physical devices with Vulkan support")));
			return;
		}

		auto& bestDevice = devices[0];
		size_t bestScore = ComputeDeviceScore(bestDevice);

		for (const auto& device : devices)
		{
			size_t score = ComputeDeviceScore(device);
			if (score > bestScore)
			{
				bestDevice = device;
				bestScore = score;
			}
		}

		physicalDevice = bestDevice;

		std::string_view deviceName = bestDevice.getProperties().deviceName;

		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>(
			"Selected physical device: {} (score: {})", 
			deviceName,
			bestScore
		)));
	}

	void PhysicalDevice::Cleanup()
	{
		
	}

	VulkanQueueFamilies PhysicalDevice::FindQueueFamilies(std::optional<vk::SurfaceKHR> _surface)
	{
		auto families = physicalDevice.getQueueFamilyProperties();

		VulkanQueueFamilies queueFamilies;

		for (uint32_t i = 0; i < families.size(); i++)
		{
			const auto& properties = families[i];

			StoreGraphicsQueueIndex(queueFamilies, properties, i);

			if (_surface.has_value())
			{
				StorePresentQueueIndex(queueFamilies, physicalDevice, *_surface, i);
			}

			StoreComputeQueueIndex(queueFamilies, properties, i);
			StoreTransferQueueIndex(queueFamilies, properties, i);

			if (queueFamilies.graphics != std::numeric_limits<uint32_t>::max() &&
				queueFamilies.compute != std::numeric_limits<uint32_t>::max() &&
				queueFamilies.transfer != std::numeric_limits<uint32_t>::max() &&
				queueFamilies.present != std::numeric_limits<uint32_t>::max())
			{
				break;
			}
		}

		QueueIndicesFallbacks(queueFamilies);

		return queueFamilies;
	}
}
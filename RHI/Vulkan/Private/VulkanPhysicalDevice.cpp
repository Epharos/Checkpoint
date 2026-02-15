#include "VulkanPhysicalDevice.hpp"

#include "VulkanInstance.hpp"

#include <Log.hpp>
#include <Macros.hpp>
#include <Profiling.hpp>
#include <RenderingHardwareInterface.hpp>

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
	}

	VulkanPhysicalDevice::VulkanPhysicalDevice(ILogger& _logger, VulkanInstance& _instance)
		: RHIPhysicalDevice(_logger), instance(_instance)
	{
		Initialize();
	}

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{
		Cleanup();
	}

	void VulkanPhysicalDevice::Initialize()
	{
		CP_PROFILE_SCOPE("VulkanPhysicalDevice#Initialize");

		auto devices = instance.GetHandle().enumeratePhysicalDevices();

		if (devices.empty())
		{
			logger.Log(CP_LOG_EVENT(cp::ILogger::Critical, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>("Failed to find any physical devices with Vulkan support")));
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

		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>(
			"Selected physical device: {} (score: {})", 
			deviceName,
			bestScore
		)));
	}

	void VulkanPhysicalDevice::Cleanup()
	{
		
	}
}
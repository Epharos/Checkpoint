#include "VulkanDevice.hpp"

#include "VulkanPhysicalDevice.hpp"
#include "VulkanQueue.hpp"

#include <Log.hpp>
#include <Macros.hpp>
#include <RenderingHardwareInterface.hpp>
#include <Profiling.hpp>

#include <set>

namespace cp
{
	VulkanDevice::VulkanDevice(ILogger& _logger, VulkanPhysicalDevice& _physicalDevice)
		: RHIDevice(_logger), physicalDevice(_physicalDevice)
	{
		Initialize();
	}

	VulkanDevice::~VulkanDevice()
	{
		Cleanup();
	}

	void VulkanDevice::Initialize()
	{
		families = physicalDevice.FindQueueFamilies();

		if (!CreateLogicalDevice())
		{
			logger.Log(CP_LOG_EVENT(cp::ILogger::Critical, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>("Failed to create logical device")));
			return;
		}

		CreateQueues();

		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>("Logical device created successfully")));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>("Graphics Queue Family: {}", families.graphics)));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>("Compute Queue Family: {}", families.compute)));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>("Transfer Queue Family: {}", families.transfer)));
	}

	void VulkanDevice::Cleanup()
	{
		if (device != VK_NULL_HANDLE)
		{
			device.destroy();
		}
	}

	bool VulkanDevice::CreateLogicalDevice()
	{
		CP_PROFILE_SCOPE("VulkanDevice#Create Logical Device");

		std::set<uint32_t> uniqueQueueFamilies = {
			families.graphics,
			families.compute,
			families.transfer,
			families.present
		};

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.setQueueCount(1);
			queueCreateInfo.setQueueFamilyIndex(queueFamily);
			queueCreateInfo.setPQueuePriorities(&queuePriority);
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// TODO : Add device features and extensions

		vk::DeviceCreateInfo createInfo;
		createInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()));
		createInfo.setPQueueCreateInfos(queueCreateInfos.data());

		if(physicalDevice.GetHandle().createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess)
		{
			return false;
		}

		return true;
	}

	void VulkanDevice::CreateQueues()
	{
		CP_PROFILE_SCOPE("VulkanDevice#Create Queues");

		auto queue = device.getQueue(families.graphics, 0);
		queues[0].push_back(std::make_unique<VulkanQueue>(queue, families.graphics, RHIQueueType::Graphics));

		queue = device.getQueue(families.compute, 0);
		queues[1].push_back(std::make_unique<VulkanQueue>(queue, families.compute, RHIQueueType::Compute));

		queue = device.getQueue(families.transfer, 0);
		queues[2].push_back(std::make_unique<VulkanQueue>(queue, families.transfer, RHIQueueType::Transfer));
	}

	RHIQueue& VulkanDevice::GetQueue(RHIQueueType _queueType, uint32_t _index)
	{
		return *queues[static_cast<size_t>(_queueType)].at(_index);
	}
}
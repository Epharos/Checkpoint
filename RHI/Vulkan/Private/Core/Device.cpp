#include "../pch.hpp"

#include "Device.hpp"

#include <Log.hpp>
#include <Profiling.hpp>

#include "PhysicalDevice.hpp"
#include "Queue.hpp"
#include "../Data/Texture.hpp"

#include <set>

namespace cp
{
	Device::Device(ILogger& _logger, PhysicalDevice& _physicalDevice)
		: IDevice(_logger), physicalDevice(_physicalDevice)
	{
		Initialize();
	}

	Device::~Device()
	{
		Cleanup();
	}

	void Device::Initialize()
	{
		families = physicalDevice.FindQueueFamilies();

		if (!CreateLogicalDevice())
		{
			logger.Log(CP_LOG_EVENT(cp::ILogger::Critical, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Failed to create logical device")));
			return;
		}

		CreateQueues();

		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Logical device created successfully")));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Graphics Queue Family: {}", families.graphics)));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Present Queue Family: {}", families.present)));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Compute Queue Family: {}", families.compute)));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Transfer Queue Family: {}", families.transfer)));
	}

	void Device::Cleanup()
	{
		if (device != VK_NULL_HANDLE)
		{
			device.waitIdle();
			device.destroy();
		}
	}

	bool Device::CreateLogicalDevice()
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

		std::vector<const char*> extensions;
		std::vector<const char*> layers;

		extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vk::DeviceCreateInfo createInfo;
		createInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()));
		createInfo.setPQueueCreateInfos(queueCreateInfos.data());
		createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
		//createInfo.setPEnabledExtensionNames(extensions.data()); 
		createInfo.ppEnabledExtensionNames = extensions.data(); // Workaround for a compiler issue with setPEnabledExtensionNames

		if(physicalDevice.GetHandle().createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess)
		{
			return false;
		}

		return true;
	}

	void Device::CreateQueues()
	{
		CP_PROFILE_SCOPE("VulkanDevice#Create Queues");

		auto queue = device.getQueue(families.graphics, 0);
		queues[0].push_back(std::make_unique<Queue>(queue, families.graphics, QueueType::Graphics));

		queue = device.getQueue(families.compute, 0);
		queues[1].push_back(std::make_unique<Queue>(queue, families.compute, QueueType::Compute));

		queue = device.getQueue(families.transfer, 0);
		queues[2].push_back(std::make_unique<Queue>(queue, families.transfer, QueueType::Transfer));
	}

	void Device::WaitIdle() const
	{
		device.waitIdle();
	}

	IQueue& Device::GetQueue(QueueType _queueType, uint32_t _index)
	{
		return *queues[static_cast<size_t>(_queueType)].at(_index);
	}

	std::shared_ptr<ITexture> Device::CreateTexture(const TextureInfo& _info)
	{
		return std::make_shared<Texture>(logger, _info, *this);
	}
}
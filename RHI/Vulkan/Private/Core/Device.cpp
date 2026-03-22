#include "../pch.hpp"

#include "Device.hpp"

#include <Common/Core/Log.hpp>
#include <Common/Core/Profiling.hpp>

#include "PhysicalDevice.hpp"
#include "Queue.hpp"
#include "../Data/Texture.hpp"
#include "../Data/Buffer.hpp"
#include "../Data/DescriptorSetLayout.hpp"
#include "../Rendering/ShaderModule.hpp"
#include "../Rendering/PipelineLayout.hpp"
#include "../Rendering/Pipeline.hpp"

#include <set>

#include "../Data/DescriptorSet.hpp"

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
		CreateCommandPools();

		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Logical device created successfully")));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Graphics Queue Family: {}", families.graphics)));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Present Queue Family: {}", families.present)));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Compute Queue Family: {}", families.compute)));
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Transfer Queue Family: {}", families.transfer)));
	}

	void Device::CleanupCommandPools() const
	{
		for (auto& commandPoolsVector : commandPools)
		{
			for (auto& commandPool : commandPoolsVector)
			{
				device.destroyCommandPool(commandPool);
			}
		}
	}

	void Device::Cleanup() const
	{
		if (device != VK_NULL_HANDLE)
		{
			device.waitIdle();

			CleanupCommandPools();

			device.destroy();
		}
	}

	vk::PhysicalDeviceDynamicRenderingFeatures Device::FillFeatureStructures()
	{
		vk::PhysicalDeviceVulkan12Features device12Features;
		device12Features.setTimelineSemaphore(true);
		device12Features.setPNext(nullptr);

		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures;
		dynamicRenderingFeatures.setDynamicRendering(true);
		dynamicRenderingFeatures.setPNext(&device12Features);

		return dynamicRenderingFeatures;
	}

	bool Device::CreateLogicalDevice()
	{
		CP_PROFILE_SCOPE_NAMED("VulkanDevice#Create Logical Device");

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
		extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

		// FEATURES

		vk::PhysicalDeviceVulkan11Features device11Features;
		device11Features.setShaderDrawParameters(true);
		device11Features.setPNext(nullptr);

		vk::PhysicalDeviceSynchronization2Features synchronization2Features;
		synchronization2Features.setSynchronization2(true);
		synchronization2Features.setPNext(&device11Features);

		vk::PhysicalDeviceVulkan12Features device12Features;
		device12Features.setTimelineSemaphore(true);
		device12Features.setPNext(&synchronization2Features);

		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures;
		dynamicRenderingFeatures.setDynamicRendering(true);
		dynamicRenderingFeatures.setPNext(&device12Features);

		// CREATION

		vk::DeviceCreateInfo createInfo;
		createInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()));
		createInfo.setPQueueCreateInfos(queueCreateInfos.data());
		createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
		createInfo.setPpEnabledExtensionNames(extensions.data());
		createInfo.setPNext(&dynamicRenderingFeatures);

		return physicalDevice.GetHandle().createDevice(&createInfo, nullptr, &device) == vk::Result::eSuccess;
	}

	void Device::CreateQueues()
	{
		CP_PROFILE_SCOPE_NAMED("VulkanDevice#Create Queues");

		auto queue = device.getQueue(families.graphics, 0);
		queues[0].push_back(std::make_unique<Queue>(queue, families.graphics, QueueType::Graphics));

		queue = device.getQueue(families.compute, 0);
		queues[1].push_back(std::make_unique<Queue>(queue, families.compute, QueueType::Compute));

		queue = device.getQueue(families.transfer, 0);
		queues[2].push_back(std::make_unique<Queue>(queue, families.transfer, QueueType::Transfer));
	}

	void Device::CreateCommandPools()
	{
		CP_PROFILE_SCOPE_NAMED("VulkanDevice#Create CommandPool");

		for (int queuesIndex = 0 ; queuesIndex < queues.size() ; queuesIndex++)
		{
			auto& queueVector = queues[queuesIndex];

			for (auto& queue : queueVector)
			{
				vk::CommandPoolCreateInfo commandPoolCreateInfo;
				commandPoolCreateInfo.setQueueFamilyIndex(queue->GetFamilyIndex());

				commandPools[queuesIndex].push_back(device.createCommandPool(commandPoolCreateInfo));
			}
		}

		CP_ENSURE_MSG(queues[0].size() == commandPools[0].size(), "There must be as much queues as there are command pools for it (graphics)");
		CP_ENSURE_MSG(queues[1].size() == commandPools[1].size(), "There must be as much queues as there are command pools for it (compute)");
		CP_ENSURE_MSG(queues[2].size() == commandPools[2].size(), "There must be as much queues as there are command pools for it (transfert)");
	}

	void Device::WaitIdle() const
	{
		device.waitIdle();
	}

	IQueue& Device::GetQueue(QueueType _queueType, uint32_t _index)
	{
		CP_EXPECT_MSG(queues[static_cast<size_t>(_queueType)].size() > _index, "index is out of bounds");
		return *queues[static_cast<size_t>(_queueType)].at(_index);
	}

	const IQueue& Device::GetQueue(QueueType _queueType, uint32_t _index) const
	{
		CP_EXPECT_MSG(queues[static_cast<size_t>(_queueType)].size() > _index, "index is out of bounds");
		return *queues[static_cast<size_t>(_queueType)][_index];
	}

	vk::CommandPool& Device::GetCommandPool(QueueType _queueType, uint32_t _index)
	{
		CP_EXPECT_MSG(commandPools[static_cast<size_t>(_queueType)].size() > _index, "index is out of bounds");
		return commandPools[static_cast<size_t>(_queueType)][_index];
	}

	const vk::CommandPool& Device::GetCommandPool(QueueType _queueType, uint32_t _index) const
	{
		CP_EXPECT_MSG(commandPools[static_cast<size_t>(_queueType)].size() > _index, "index is out of bounds");
		return commandPools[static_cast<size_t>(_queueType)][_index];
	}

	std::shared_ptr<ITexture> Device::CreateTexture(const TextureInfo& _info)
	{
		return std::make_shared<Texture>(logger, _info, *this);
	}

	std::shared_ptr<IBuffer> Device::CreateBuffer(const BufferInfo& _info)
	{
		return std::make_shared<Buffer>(logger, _info, *this);
	}

	std::shared_ptr<IShaderModule> Device::CreateShaderModule(const ShaderModuleInfo& _info)
	{
		return std::make_shared<ShaderModule>(_info, *this);
	}

	std::shared_ptr<IDescriptorSetLayout> Device::CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& _info)
	{
		return std::make_shared<DescriptorSetLayout>(_info, *this);
	}

	std::shared_ptr<IPipelineLayout> Device::CreatePipelineLayout(const PipelineLayoutInfo& _info)
	{
		return std::make_shared<PipelineLayout>(_info, *this);
	}

	std::shared_ptr<IDescriptorSet> Device::CreateDescriptorSet(const IDescriptorSetLayout &_descriptorSetLayout)
	{
		return std::make_shared<DescriptorSet>(_descriptorSetLayout, *this);
	}

	std::shared_ptr<IPipeline> Device::CreateGraphicsPipeline(const GraphicsPipelineInfo& _info)
	{
		return std::make_shared<Pipeline>(_info, *this);
	}

	std::shared_ptr<IPipeline> Device::CreateComputePipeline(const ComputePipelineInfo& _info)
	{
		return std::make_shared<Pipeline>(_info, *this);
	}
}

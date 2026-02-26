#include "pch.hpp"

#include "VulkanRHI.hpp"

#include <memory>

#include "Core/VulkanSurface.hpp"
#include "Core/VulkanSwapchain.hpp"

#include <Profiling.hpp>

namespace cp
{
	namespace
	{
		bool RetrieveVulkanVersion(ILogger& _logger)
		{
			uint32_t vulkanVersion = 0;

			if (vk::enumerateInstanceVersion(&vulkanVersion) != vk::Result::eSuccess)
			{
				_logger.Log(CP_LOG_EVENT(cp::ILogger::Error, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Couldn't retrieve Vulkan version")));
				return false;
			}

			std::stringstream ss;
			ss << VK_VERSION_MAJOR(vulkanVersion) << "." << VK_VERSION_MINOR(vulkanVersion) << "." << VK_VERSION_PATCH(vulkanVersion);

			_logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Vulkan version " + ss.str())));
			return true;
		}
	}

	VulkanRHI::VulkanRHI(std::shared_ptr<cp::ILogger> _logger)
		: RenderingHardwareInterface(_logger)
	{
		if(!RetrieveVulkanVersion(GetLogger()))
			throw std::runtime_error("Could not locate Vulkan on the system.");

		_logger->Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Vulkan RHI created.")));
	}

	IInstance& VulkanRHI::CreateInstance(const InstanceInfo& _info)
	{
		CP_EXPECT_MSG(!instance, "Instance already created");
		CP_PROFILE_SCOPE("Vulkan#Instance creation");
		instance = std::make_unique<cp::VulkanInstance>(GetLogger(), _info);
		CP_ENSURE_MSG(instance, "Failed to create Vulkan instance");
		return *instance;
	}

	IPhysicalDevice& VulkanRHI::CreatePhysicalDevice()
	{
		CP_EXPECT_MSG(instance, "Instance must be created before creating a physical device");
		CP_EXPECT_MSG(!physicalDevice, "Physical device already created");
		CP_PROFILE_SCOPE("Vulkan#PhysicalDevice creation");
		physicalDevice = std::make_unique<VulkanPhysicalDevice>(GetLogger(), *instance);
		CP_ENSURE_MSG(physicalDevice, "Failed to create Vulkan physical device");
		return *physicalDevice;
	}

	IDevice& VulkanRHI::CreateDevice()
	{
		CP_EXPECT_MSG(physicalDevice, "Physical device must be created before creating a logical device");
		CP_EXPECT_MSG(!device, "Logical device already created");
		CP_PROFILE_SCOPE("Vulkan#Device creation");
		device = std::make_unique<VulkanDevice>(GetLogger(), *physicalDevice);
		CP_ENSURE_MSG(device, "Failed to create Vulkan logical device");
		return *device;
	}

	std::unique_ptr<VulkanSurface> VulkanRHI::CreateSurface(const SurfaceInfo& _info)
	{
		CP_EXPECT_MSG(instance, "Instance must be created before creating a surface");
		CP_PROFILE_SCOPE("Vulkan#Surface creation");
		auto surface = std::make_unique<VulkanSurface>(GetLogger(), _info, *instance);
		CP_ENSURE_MSG(surface, "Failed to create Vulkan surface");
		return surface;
	}

	std::unique_ptr<ISwapchain> VulkanRHI::CreateSwapchain(const SwapchainInfo& _info)
	{
		CP_EXPECT_MSG(device, "Logical device must be created before creating a swapchain");
		CP_PROFILE_SCOPE("Vulkan#Swapchain creation");
		auto swapchain = std::make_unique<VulkanSwapchain>(GetLogger(), _info, *device);
		CP_ENSURE_MSG(swapchain, "Failed to create Vulkan swapchain");
		return swapchain;
	}

	IInstance& VulkanRHI::GetInstance()
	{
		CP_EXPECT_MSG(instance, "Instance must be created before it can be accessed");
		return *instance;
	}

	const IInstance& VulkanRHI::GetInstance() const
	{
		CP_EXPECT_MSG(instance, "Instance must be created before it can be accessed");
		return *instance;
	}

	IPhysicalDevice& VulkanRHI::GetPhysicalDevice()
	{
		CP_EXPECT_MSG(physicalDevice, "Physical device must be created before it can be accessed");
		return *physicalDevice;
	}

	const IPhysicalDevice& VulkanRHI::GetPhysicalDevice() const
	{
		CP_EXPECT_MSG(physicalDevice, "Physical device must be created before it can be accessed");
		return *physicalDevice;
	}

	IDevice& VulkanRHI::GetDevice()
	{
		CP_EXPECT_MSG(device, "Logical device must be created before it can be accessed");
		return *device;
	}

	const IDevice& VulkanRHI::GetDevice() const
	{
		CP_EXPECT_MSG(device, "Logical device must be created before it can be accessed");
		return *device;
	}
}
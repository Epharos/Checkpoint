#include "VulkanRHI.hpp"

#include "VulkanInstance.hpp"

namespace cp
{
	namespace
	{
		bool RetrieveVulkanVersion(ILogger& _logger)
		{
			uint32_t vulkanVersion = 0;

			if (vk::enumerateInstanceVersion(&vulkanVersion) != vk::Result::eSuccess)
			{
				_logger.Log(CP_LOG_EVENT(cp::LogLevel::Error, cp::Message::Create<cp::TextComponent>("Couldn't retrieve Vulkan version")));
				return false;
			}

			std::stringstream ss;
			ss << VK_VERSION_MAJOR(vulkanVersion) << "." << VK_VERSION_MINOR(vulkanVersion) << "." << VK_VERSION_PATCH(vulkanVersion);

			_logger.Log(CP_LOG_EVENT(cp::LogLevel::Info, cp::Message::Create<cp::TextComponent>("Vulkan version " + ss.str())));
			return true;
		}
	}

	VulkanRHI::VulkanRHI(std::shared_ptr<cp::ILogger> _logger)
		: RenderingHardwareInterface(_logger)
	{
		if(!RetrieveVulkanVersion(GetLogger()))
			throw std::runtime_error("Could not locate Vulkan on the system.");

		_logger->Log(CP_LOG_EVENT(cp::LogLevel::Info, cp::Message::Create<cp::TextComponent>("Vulkan RHI created.")));
	}

	std::unique_ptr<RHIInstance> VulkanRHI::CreateInstance(const RHIInstanceInfo& _info)
	{
		return std::make_unique<VulkanInstance>(GetLogger(), _info);
	}
}
#include "VulkanInstance.hpp"

#include <sstream>

#include "Macros.hpp"
#include "Log.hpp"

namespace cp
{
	VulkanInstance::VulkanInstance(ILogger& _logger, const RHIInstanceInfo& _info)
		: RHIInstance(_logger, _info)
	{
		Initialize();
	}

	VulkanInstance::~VulkanInstance()
	{
		Cleanup();
	}

	vk::Instance& VulkanInstance::GetInstance()
	{
		return instance;
	}

	const vk::Instance& VulkanInstance::GetInstance() const
	{
		return instance;
	}

	void VulkanInstance::Initialize()
	{
		if (!CreateInstance())
		{
			logger.Log(CP_LOG_EVENT(cp::LogLevel::Critical, cp::Message::Create<cp::TextComponent>("Failed to create instance")));
			return;
		}

		logger.Log(CP_LOG_EVENT(cp::LogLevel::Info, cp::Message::Create<cp::TextComponent>("Instance created successfully")));

		if (info.enableValidationLayers)
		{
			if (!CreateDebugMessenger())
			{
				logger.Log(CP_LOG_EVENT(cp::LogLevel::Error, cp::Message::Create<cp::TextComponent>("Failed to create debug messenger")));
				return;
			}

			logger.Log(CP_LOG_EVENT(cp::LogLevel::Info, cp::Message::Create<cp::TextComponent>("Debug messenger created successfully")));
		}
	}

	void VulkanInstance::Cleanup()
	{
		if (info.enableValidationLayers)
		{
			instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dispatchLoaderDynamic);
		}

		instance.destroy();
	}

	void VulkanInstance::ValidateExtensionsAndLayers(
		const std::vector<const char*>& _extensions, 
		const std::vector<const char*>& _layers
	) const
	{
		// Validate Extensions
		{
			auto availableExtensions = vk::enumerateInstanceExtensionProperties();

			for (const auto& extension : _extensions)
			{
				bool found = false;
				for (const auto& availableExtension : availableExtensions)
				{
					if (strcmp(extension, availableExtension.extensionName) == 0)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					logger.Log(CP_LOG_EVENT(cp::LogLevel::Error, cp::Message::Create<cp::TextComponent>("{} extension not found", extension)));
				}
			}
		}

		// Validate Layers
		{
			auto availableLayers = vk::enumerateInstanceLayerProperties();
			for (const auto& layer : _layers)
			{
				bool found = false;
				for (const auto& availableLayer : availableLayers)
				{
					if (strcmp(layer, availableLayer.layerName) == 0)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					logger.Log(CP_LOG_EVENT(cp::LogLevel::Error, cp::Message::Create<cp::TextComponent>("{} layer not found", layer)));
				}
			}
		}
	}

	bool VulkanInstance::CreateInstance()
	{
		vk::ApplicationInfo appInfo;
		appInfo.setPApplicationName(info.appName.c_str()); // User defined
		appInfo.setApplicationVersion(info.appVersion); // User defined
		appInfo.setApiVersion(info.apiVersion == 0 ? VK_API_VERSION_1_4 : info.apiVersion);
		appInfo.setPEngineName("Checkpoint");
		appInfo.setEngineVersion(CP_VERSION);

		std::vector<const char*> extensions;
		std::vector<const char*> layers;

		if (info.enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		ValidateExtensionsAndLayers(extensions, layers);

		vk::InstanceCreateInfo instanceInfo;
		instanceInfo.setPApplicationInfo(&appInfo);
		instanceInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
		instanceInfo.setPpEnabledExtensionNames(extensions.data());
		instanceInfo.setEnabledLayerCount(static_cast<uint32_t>(layers.size()));
		instanceInfo.setPpEnabledLayerNames(layers.data());

		instance = vk::createInstance(instanceInfo);

		return instance != VK_NULL_HANDLE;
	}

	bool VulkanInstance::CreateDebugMessenger()
	{
		dispatchLoaderDynamic = vk::detail::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

		vk::PFN_DebugUtilsMessengerCallbackEXT callback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(cp::VulkanInstance::DebugLayerCallback);

		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		debugCreateInfo.setMessageSeverity(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
		);

		debugCreateInfo.setMessageType(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
		);

		debugCreateInfo.setPfnUserCallback(callback);
		debugCreateInfo.setPUserData(&logger);

		debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, dispatchLoaderDynamic);

		return debugMessenger != VK_NULL_HANDLE;
	}

	VKAPI_ATTR vk::Bool32 VKAPI_PTR VulkanInstance::DebugLayerCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT _messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT _messageType, const vk::DebugUtilsMessengerCallbackDataEXT* _callbackData, void* _userData)
	{
		ILogger* logger = reinterpret_cast<ILogger*>(_userData);

		std::stringstream message;
		message << "[VL ";
		if (_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
			message << "GENERAL";
		else if (_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
			message << "VALIDATION";
		else if (_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
			message << "PERFORMANCE";

		message << "] ";
		message << _callbackData->pMessage;

		switch (_messageSeverity)
		{
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			logger->Log(CP_LOG_EVENT(cp::LogLevel::Error, cp::Message::Create<cp::TextComponent>(message.str())));
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			logger->Log(CP_LOG_EVENT(cp::LogLevel::Warning, cp::Message::Create<cp::TextComponent>(message.str())));
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			logger->Log(CP_LOG_EVENT(cp::LogLevel::Info, cp::Message::Create<cp::TextComponent>(message.str())));
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			logger->Log(CP_LOG_EVENT(cp::LogLevel::Debug, cp::Message::Create<cp::TextComponent>(message.str())));
			break;
		}

		return VK_FALSE;
	}
}
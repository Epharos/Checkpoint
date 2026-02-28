#include "../pch.hpp"

#include "Instance.hpp"

#include <sstream>

#include <Macros.hpp>
#include <Log.hpp>
#include <Profiling.hpp>

#include "RenderingHardwareInterface.hpp"
#include "PhysicalDevice.hpp"
#include "Surface.hpp"

namespace cp
{
	Instance::Instance(ILogger& _logger, const InstanceInfo& _instanceInfo)
		: IInstance(_logger, _instanceInfo)
	{
		Initialize();
	}

	Instance::~Instance()
	{
		Cleanup();
	}

	vk::Instance& Instance::GetHandle()
	{
		return instance;
	}

	const vk::Instance& Instance::GetHandle() const
	{
		return instance;
	}

	void Instance::Initialize()
	{
		if (!CreateInstance())
		{
			logger.Log(CP_LOG_EVENT(cp::ILogger::Critical, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Failed to create instance")));
			return;
		}

		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Instance created successfully")));

		if (instanceInfo.enableValidationLayers)
		{
			if (!CreateDebugMessenger())
			{
				logger.Log(CP_LOG_EVENT(cp::ILogger::Error, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Failed to create debug messenger")));
				return;
			}

			logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Debug messenger created successfully")));
		}
	}

	void Instance::Cleanup()
	{
		if (instanceInfo.enableValidationLayers)
		{
			instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dispatchLoaderDynamic);
		}

		instance.destroy();
	}

	void Instance::ValidateExtensionsAndLayers(
		const std::vector<const char*>& _extensions, 
		const std::vector<const char*>& _layers
	) const
	{
		// Validate Extensions
		{
			CP_PROFILE_SCOPE("VulkanInstance#Validate Extensions");
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
					logger.Log(CP_LOG_EVENT(cp::ILogger::Error, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>("{} extension not found", extension)));
				}
			}
		}

		// Validate Layers
		{
			CP_PROFILE_SCOPE("VulkanInstance#Validate Layers");
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
					logger.Log(CP_LOG_EVENT(cp::ILogger::Error, cp::RHI::RHI_Label, cp::Message::Create<cp::TextComponent>("{} layer not found", layer)));
				}
			}
		}
	}

	bool Instance::CreateInstance()
	{
		vk::ApplicationInfo appInfo;
		appInfo.setPApplicationName(instanceInfo.appName.c_str()); // User defined
		appInfo.setApplicationVersion(instanceInfo.appVersion); // User defined
		appInfo.setApiVersion(instanceInfo.apiVersion == 0 ? VK_API_VERSION_1_4 : instanceInfo.apiVersion);
		appInfo.setPEngineName("Checkpoint");
		appInfo.setEngineVersion(CP_VERSION);

		std::vector<const char*> extensions;
		std::vector<const char*> layers;

		if (instanceInfo.enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(CP_PLATFORM_WINDOWS)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		ValidateExtensionsAndLayers(extensions, layers);

		vk::InstanceCreateInfo instanceInfo;
		instanceInfo.setPApplicationInfo(&appInfo);
		instanceInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
		instanceInfo.setPpEnabledExtensionNames(extensions.data());
		instanceInfo.setEnabledLayerCount(static_cast<uint32_t>(layers.size()));
		instanceInfo.setPpEnabledLayerNames(layers.data());

		{ 
			CP_PROFILE_SCOPE("VulkanInstance#Create instance"); 
			instance = vk::createInstance(instanceInfo);
		}

		return instance != VK_NULL_HANDLE;
	}

	bool Instance::CreateDebugMessenger()
	{
		CP_PROFILE_SCOPE("VulkanInstance#Create Debug Messenger");
		dispatchLoaderDynamic = vk::detail::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

		vk::PFN_DebugUtilsMessengerCallbackEXT callback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(cp::Instance::DebugLayerCallback);

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

	VKAPI_ATTR vk::Bool32 VKAPI_PTR Instance::DebugLayerCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT _messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT _messageType, const vk::DebugUtilsMessengerCallbackDataEXT* _callbackData, void* _userData)
	{
		ILogger* logger = reinterpret_cast<ILogger*>(_userData);

		std::stringstream message;
		if (_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
			message << "GENERAL";
		else if (_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
			message << "VALIDATION";
		else if (_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
			message << "PERFORMANCE";

		message << " > ";
		message << _callbackData->pMessage;

		switch (_messageSeverity)
		{
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			logger->Log(CP_LOG_EVENT(cp::ILogger::Error, "Validation", cp::Message::Create<cp::TextComponent>(message.str())));
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "Validation", cp::Message::Create<cp::TextComponent>(message.str())));
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Validation", cp::Message::Create<cp::TextComponent>(message.str())));
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			logger->Log(CP_LOG_EVENT(cp::ILogger::Debug, "Validation", cp::Message::Create<cp::TextComponent>(message.str())));
			break;
		}

		return VK_FALSE;
	}
}
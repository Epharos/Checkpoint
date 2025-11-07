#include "pch.hpp"
#include "VulkanContext.hpp"

VKAPI_ATTR vk::Bool32 VKAPI_PTR DebugLayerCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT _messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT _messageType, const vk::DebugUtilsMessengerCallbackDataEXT* _callbackData, void* _userData);

void cp::VulkanContext::Initialize(VulkanContextInfo& _contextInfo)
{
	LOG_TRACE("Checkpoint (" + VersionToString(ENGINE_VERSION) + ")");

	uint32 vulkanVersion = 0;
	if(vk::enumerateInstanceVersion(&vulkanVersion) != vk::Result::eSuccess)
		LOG_ERROR("Couldn't retrieve Vulkan version");

	LOG_TRACE("Vulkan version: " + VersionToString(vulkanVersion));

	uint32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32 i = 0; i < glfwExtensionCount; i++)
		_contextInfo.extensions.instanceExtensions.push_back(glfwExtensions[i]);

	#ifdef USE_DEBUG_LAYER
	_contextInfo.extensions.instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	_contextInfo.extensions.instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	#endif

#ifdef IN_EDITOR
	instance = _contextInfo.instance;
#endif

#ifndef IN_EDITOR
	CreateInstance(vulkanVersion, _contextInfo.appName, _contextInfo.appVersion, _contextInfo.extensions);
#endif
	CreateDebugMessenger();
	PickPhysicalDevice();

#ifndef IN_EDITOR
	CreateSurface();
#endif

	CreateLogicalDevice();
	CreateCommandPool();

	pipelinesManager = new cp::PipelinesManager(GetDevice());
	layoutsManager = new cp::LayoutsManager(GetDevice());
	descriptorSetLayoutsManager = new cp::DescriptorSetLayoutsManager(GetDevice());
	descriptorSetManager = new cp::DescriptorSetManager(GetDevice());
}

void cp::VulkanContext::Shutdown()
{
	LOG_TRACE("Shutting down Vulkan context");

	device.destroyCommandPool(commandPool);

	pipelinesManager->Cleanup();
	layoutsManager->Cleanup();
	descriptorSetLayoutsManager->Cleanup();
	descriptorSetManager->Cleanup();

	device.destroy();
	instance.destroySurfaceKHR(surface);

	#ifdef USE_DEBUG_LAYER
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dynamicLoader);
	#endif

	instance.destroy();

	delete pipelinesManager;
	delete layoutsManager;
	delete descriptorSetLayoutsManager;
	delete descriptorSetManager;
}

std::string cp::VulkanContext::VersionToString(const uint32& _version)
{
	std::stringstream ss;
	ss << VK_VERSION_MAJOR(_version) << "." << VK_VERSION_MINOR(_version) << "." << VK_VERSION_PATCH(_version);
	return ss.str();
}

void cp::VulkanContext::CreateInstance(const uint32& _vulkanVersion, const std::string& _appName, const uint32& _appVersion, const VulkanExtensions& _extensions)
{
	vk::ApplicationInfo appInfo(_appName.c_str(), _appVersion, "Checkpoint", ENGINE_VERSION, _vulkanVersion);

	ValidateExtensions(_extensions);

	vk::InstanceCreateInfo instanceInfo({}, &appInfo, _extensions.instanceLayers, _extensions.instanceExtensions);

	instance = vk::createInstance(instanceInfo);
}

void cp::VulkanContext::PickPhysicalDevice()
{
	auto physicalDevices = instance.enumeratePhysicalDevices();

	if (physicalDevices.size() == 0)
	{
		LOG_ERROR("No physical devices found");
		return;
	}

	for (const auto& device : physicalDevices)
	{
		vk::PhysicalDeviceProperties properties = device.getProperties();
		
		if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			physicalDevice = device;
			LOG_INFO("Picked physical device: " + std::string(properties.deviceName.data()));
			break;
		}
	}

#ifdef PRINT_VULKAN_SPECS

	vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

	LOG_DEBUG(MF(	"PHYSICAL DEVICE PROPERTIES\n",
					"Min Buffer Offset Alignment (Uniform): ", properties.limits.minUniformBufferOffsetAlignment, "\n",
					"Min Buffer Offset Alignment (Storage): ", properties.limits.minStorageBufferOffsetAlignment));

#endif
}

void cp::VulkanContext::CreateLogicalDevice()
{
	queueFamilyIndices = FindQueueFamilies(physicalDevice, surface);

	std::vector<uint32> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value() };
	if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value())
		uniqueQueueFamilies.push_back(queueFamilyIndices.presentFamily.value());

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;

	for (uint32 queueFamily : uniqueQueueFamilies)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo({}, queueFamily, 1, &queuePriority);
		queueCreateInfos.push_back(queueCreateInfo);
	}

	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> deviceLayers = {};

	#if _DEBUG
	deviceLayers.push_back("VK_LAYER_KHRONOS_validation");
	#endif

	vk::PhysicalDeviceVulkan12Features v12features;
	v12features.shaderOutputLayer = VK_TRUE;

	vk::PhysicalDeviceFeatures2 features2;
	features2.features.samplerAnisotropy = VK_TRUE;
	features2.features.tessellationShader = VK_TRUE;
	features2.features.geometryShader = VK_TRUE;
	features2.pNext = &v12features;

	vk::DeviceCreateInfo deviceInfo({}, 
		static_cast<uint32>(queueCreateInfos.size()), queueCreateInfos.data(), 
		static_cast<uint32>(deviceLayers.size()), deviceLayers.data(),
		static_cast<uint32>(deviceExtensions.size()), deviceExtensions.data(),
		nullptr, &features2);

	device = physicalDevice.createDevice(deviceInfo);
}

void cp::VulkanContext::CreateSurface()
{
	
}

void cp::VulkanContext::CreateDebugMessenger()
{
	dynamicLoader = vk::detail::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

	vk::PFN_DebugUtilsMessengerCallbackEXT callback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(DebugLayerCallback);

	#ifdef USE_DEBUG_LAYER
	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo(
		vk::DebugUtilsMessengerCreateFlagsEXT(),
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
		DebugLayerCallback, nullptr);

	debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, dynamicLoader);
	#endif
}

void cp::VulkanContext::CreateCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndices.graphicsFamily.value());
	commandPool = device.createCommandPool(poolInfo);
}

void cp::VulkanContext::ValidateExtensions(const VulkanExtensions& _extensions) const
{
	auto availableExtensions = vk::enumerateInstanceExtensionProperties();

	size_t validatedCount = _extensions.instanceExtensions.size();

	for (const auto& extension : _extensions.instanceExtensions)
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
			validatedCount--;
			LOG_ERROR("Extension " + std::string(extension) + " not supported");
		}
	}

	LOG_INFO("Validated " + std::to_string(validatedCount) + "/" + std::to_string(_extensions.instanceExtensions.size()) + " extensions");

	auto availableLayers = vk::enumerateInstanceLayerProperties();

	validatedCount = _extensions.instanceLayers.size();

	for (const auto& layer : _extensions.instanceLayers)
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
			validatedCount--;
			LOG_ERROR("Layer " + std::string(layer) + " not supported");
		}
	}

	LOG_INFO("Validated " + std::to_string(validatedCount) + "/" + std::to_string(_extensions.instanceLayers.size()) + " layers");
}

VKAPI_ATTR vk::Bool32 VKAPI_PTR DebugLayerCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT _messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT _messageType, const vk::DebugUtilsMessengerCallbackDataEXT* _callbackData, void* _userData)
{
	std::string message = "[VL ";
	if(_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
		message += "GENERAL";
	else if (_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
		message += "VALIDATION";
	else if (_messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		message += "PERFORMANCE";

	message += "] ";
	message += _callbackData->pMessage;

	switch (_messageSeverity)
	{
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
		LOG_ERROR(message);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
		LOG_WARNING(message);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
		LOG_INFO(message);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
		LOG_TRACE(message);
		break;
	}

	return VK_FALSE;
}
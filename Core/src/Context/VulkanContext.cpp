#include "pch.hpp"
#include "VulkanContext.hpp"

#include "Devices.hpp"

#define ENGINE_VERSION VK_MAKE_API_VERSION(0, 1, 0, 0)

void Context::VulkanContext::Initialize(VulkanContextInfo& _contextInfo)
{
	LOG_TRACE("Checkpoint (" + VersionToString(ENGINE_VERSION) + ")");

	uint32 vulkanVersion = 0;
	if(vk::enumerateInstanceVersion(&vulkanVersion) != vk::Result::eSuccess)
		LOG_ERROR("Couldn't retrieve Vulkan version");

	LOG_TRACE("Vulkan version: " + VersionToString(vulkanVersion));

	CreateWindow();

	uint32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32 i = 0; i < glfwExtensionCount; i++)
		_contextInfo.extensions.instanceExtensions.push_back(glfwExtensions[i]);

	_contextInfo.extensions.instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if _DEBUG
	_contextInfo.extensions.instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	_contextInfo.extensions.instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

	CreateInstance(vulkanVersion, _contextInfo.appName, _contextInfo.appVersion, _contextInfo.extensions);
	PickPhysicalDevice();
	CreateSurface();
	CreateLogicalDevice();
}

void Context::VulkanContext::Shutdown()
{
	LOG_TRACE("Shutting down Vulkan context");
	device.destroy();
	instance.destroySurfaceKHR(surface);
	instance.destroy();
}

std::string Context::VulkanContext::VersionToString(const uint32& _version) const
{
	std::stringstream ss;
	ss << VK_VERSION_MAJOR(_version) << "." << VK_VERSION_MINOR(_version) << "." << VK_VERSION_PATCH(_version);
	return ss.str();
}

void Context::VulkanContext::CreateInstance(const uint32& _vulkanVersion, const std::string& _appName, const uint32& _appVersion, const VulkanExtensions& _extensions)
{
	vk::ApplicationInfo appInfo(_appName.c_str(), _appVersion, "Checkpoint", ENGINE_VERSION, _vulkanVersion);

	ValidateExtensions(_extensions);

	vk::InstanceCreateInfo instanceInfo({}, &appInfo, _extensions.instanceLayers, _extensions.instanceExtensions);

	instance = vk::createInstance(instanceInfo);
}

void Context::VulkanContext::PickPhysicalDevice()
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
}

void Context::VulkanContext::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

	std::vector<uint32> uniqueQueueFamilies = { indices.graphicsFamily.value() };
	if (indices.graphicsFamily.value() != indices.presentFamily.value())
		uniqueQueueFamilies.push_back(indices.presentFamily.value());

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

	vk::PhysicalDeviceFeatures deviceFeatures; // Empty for now, but add samplerAnisotropy, tesselationShader, etc.

	vk::DeviceCreateInfo deviceInfo({}, 
		static_cast<uint32>(queueCreateInfos.size()), queueCreateInfos.data(), 
		static_cast<uint32>(deviceLayers.size()), deviceLayers.data(),
		static_cast<uint32>(deviceExtensions.size()), deviceExtensions.data(),
		&deviceFeatures);

	device = physicalDevice.createDevice(deviceInfo);
}

void Context::VulkanContext::CreateWindow()
{
	if (!glfwInit())
	{
		LOG_FATAL("Failed to initialize GLFW");
		return;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(800, 600, "Checkpoint", nullptr, nullptr);
}

void Context::VulkanContext::CreateSurface()
{
	VkSurfaceKHR surfaceHandle;
	VkResult vr = glfwCreateWindowSurface(instance, window, nullptr, &surfaceHandle);

	if (vr != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create window surface " + vr);
		return;
	}

	LOG_DEBUG("Created window surface");
	surface = surfaceHandle;
}

void Context::VulkanContext::ValidateExtensions(const VulkanExtensions& _extensions) const
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

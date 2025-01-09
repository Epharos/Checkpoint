#pragma once

#include "../pch.hpp"
#include "Platform.hpp"

#include "Devices.hpp"

typedef uint32_t uint32;

namespace Context
{
	struct VulkanExtensions
	{
		std::vector<const char*> instanceExtensions;
		std::vector<const char*> instanceLayers;
	};

	struct VulkanContextInfo
	{
		std::string appName = "App";
		uint32 appVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

		Platform* platform = nullptr;

		VulkanExtensions extensions = {};
	};

	struct QueueFamilyIndices;

	class VulkanContext
	{
	public:
		VulkanContext() = default;
		void Initialize(VulkanContextInfo& _contextInfo);
		void Shutdown();

#pragma region Getters
		inline constexpr vk::Instance GetInstance() const { return instance; }
		inline constexpr vk::PhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
		inline constexpr vk::Device GetDevice() const { return device; }
		inline constexpr vk::SurfaceKHR GetSurface() const { return surface; }
		inline constexpr Platform* GetPlatform() const { return platform; }
		inline constexpr vk::CommandPool GetCommandPool() const { return commandPool; }
		inline constexpr QueueFamilyIndices GetQueueFamilyIndices() const { return queueFamilyIndices; }
		inline constexpr vk::DispatchLoaderDynamic GetDynamicLoader() const { return dynamicLoader; }
#pragma endregion

	private:
#pragma region Variables
		Platform* platform;

		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::SurfaceKHR surface;

		vk::DispatchLoaderDynamic dynamicLoader;

		#ifdef USE_DEBUG_LAYER
		vk::DebugUtilsMessengerEXT debugMessenger;
		#endif

		QueueFamilyIndices queueFamilyIndices;

		vk::CommandPool commandPool;
#pragma endregion

#pragma region Context Creation
		void CreateInstance(const uint32& _vulkanVersion, const std::string& _appName, const uint32& _appVersion, const VulkanExtensions& _extensions);
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSurface();
		void CreateDebugMessenger();
		void CreateCommandPool();
#pragma endregion

#pragma region Helper Functions
		void ValidateExtensions(const VulkanExtensions& _extensions) const;

		std::string VersionToString(const uint32& _version) const;
#pragma endregion
	};
}
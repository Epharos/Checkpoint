#pragma once

#define PRINT_VULKAN_SPECS

#define ENGINE_VERSION VK_MAKE_API_VERSION(0, 1, 0, 0)

#include "../pch.hpp"
#include "Platforms/Platform.hpp"

#include "Devices.hpp"

#include "../Render/Pipeline/PipelinesManager.hpp"
#include "../Render/Pipeline/LayoutsManager.hpp"
#include "../Render/Pipeline/SetLayoutsManager.hpp"
#include "../Render/Pipeline/DescriptorSetManager.hpp"

typedef uint32_t uint32;

namespace cp
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

		VulkanExtensions extensions = {};

#ifdef IN_EDITOR
		vk::Instance instance = nullptr;
#endif
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
		inline constexpr vk::CommandPool GetCommandPool() const { return commandPool; }
		inline constexpr QueueFamilyIndices GetQueueFamilyIndices() const { return queueFamilyIndices; }
		inline constexpr vk::detail::DispatchLoaderDynamic GetDynamicLoader() const { return dynamicLoader; }
		inline cp::PipelinesManager* GetPipelinesManager() const { return pipelinesManager; }
		inline cp::LayoutsManager* GetLayoutsManager() const { return layoutsManager; }
		inline cp::DescriptorSetLayoutsManager* GetDescriptorSetLayoutsManager() const { return descriptorSetLayoutsManager; }
		inline cp::DescriptorSetManager* GetDescriptorSetManager() const { return descriptorSetManager; }
		inline constexpr vk::DescriptorPool GetDescriptorPool() const { return descriptorSetManager->GetDescriptorPool(); }

		static std::string VersionToString(const uint32& _version);
#pragma endregion

	private:
#pragma region Variables
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;

		vk::detail::DispatchLoaderDynamic dynamicLoader;

		#ifdef USE_DEBUG_LAYER
		vk::DebugUtilsMessengerEXT debugMessenger;
		#endif

		QueueFamilyIndices queueFamilyIndices;

		vk::CommandPool commandPool;

		cp::PipelinesManager* pipelinesManager;
		cp::LayoutsManager* layoutsManager;
		cp::DescriptorSetLayoutsManager* descriptorSetLayoutsManager;
		cp::DescriptorSetManager* descriptorSetManager;
#pragma endregion

#pragma region Context Creation
		void CreateInstance(const uint32& _vulkanVersion, const std::string& _appName, const uint32& _appVersion, const VulkanExtensions& _extensions);
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateDebugMessenger();
		void CreateCommandPool();
#pragma endregion

#pragma region Helper Functions
		void ValidateExtensions(const VulkanExtensions& _extensions) const;
#pragma endregion
	};
}
#pragma once

#include "../pch.hpp"


#include <IInstance.hpp>

namespace cp
{
	class VulkanInstance final : public IInstance
	{
	public:
		VulkanInstance(ILogger& _logger, const InstanceInfo& _info);
		~VulkanInstance() override;

		[[nodiscard]] vk::Instance& GetHandle();
		[[nodiscard]] const vk::Instance& GetHandle() const;

	private:
		void Initialize();
		void Cleanup();

		void ValidateExtensionsAndLayers(const std::vector<const char*>& _extensions, const std::vector<const char*>& _layers) const;

		bool CreateInstance();
		bool CreateDebugMessenger();

		static VKAPI_ATTR vk::Bool32 VKAPI_PTR DebugLayerCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT _messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* _callbackData,
			void* _userData);

	private:
		vk::Instance instance{ VK_NULL_HANDLE };

		vk::detail::DispatchLoaderDynamic dispatchLoaderDynamic;
		vk::DebugUtilsMessengerEXT debugMessenger;
	};
}
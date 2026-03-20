#pragma once

#include "../pch.hpp"

#include <RHI/Core.hpp>

namespace cp
{
	class Instance final : public IInstance
	{
	public:
		Instance(ILogger& _logger, const InstanceInfo& _info);
		~Instance() override;

		[[nodiscard]] vk::Instance& GetHandle();
		[[nodiscard]] const vk::Instance& GetHandle() const;

		[[nodiscard]] vk::detail::DispatchLoaderDynamic& GetDispatchLoaderDynamic() { return dispatchLoaderDynamic; }
	private:
		void Initialize();
		void Cleanup() const;

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
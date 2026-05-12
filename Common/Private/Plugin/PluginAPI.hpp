#pragma once

#include <cstdint>
#include <string_view>

#include "../Log/ILogger.hpp"
#include "../Core/Registry.hpp"

#include "../../Public/Common/Core/Macros.hpp"
#include "../../Public/Common/Plugin/IEditorStateProvider.hpp"
#include "../../Public/Common/Plugin/IViewportController.hpp"
#include "../../Public/Common/Plugin/IKeybindRegistrar.hpp"

namespace cp
{
	class RegistryManager;
	class AssetRegistry;

	struct PluginRuntimeContext
	{
		ILogger* mainLogger = nullptr;
		RegistryManager* registryManager = nullptr;
		AssetRegistry* assetRegistry = nullptr;
	};

	struct PluginEditorContext
	{
		ILogger* mainLogger = nullptr;
		RegistryManager* registryManager = nullptr;
		AssetRegistry* assetRegistry = nullptr;
		IEditorStateProvider* editorState = nullptr;
		IViewportController* viewportController = nullptr;
		IKeybindRegistrar* keybindRegistrar = nullptr;
	};

	using PluginRegisterRuntimeFn = bool(*)(PluginRuntimeContext& _context);
	using PluginShutdownRuntimeFn = void(*)(PluginRuntimeContext& _context);
	using PluginRegisterEditorFn = bool(*)(PluginEditorContext& _context);
	using PluginShutdownEditorFn = void(*)(PluginEditorContext& _context);

	enum class PluginHostLoadProfile : uint8_t
	{
		RuntimeOnly,
		RuntimeThenEditor
	};

	struct PluginHostContext
	{
		PluginHostLoadProfile loadProfile = PluginHostLoadProfile::RuntimeOnly;
		PluginRuntimeContext runtimeContext{};
		PluginEditorContext editorContext{};
	};

	struct PluginDescriptor
	{
		uint32_t apiVersion = 0;
		const char* name = nullptr;
		PluginRegisterRuntimeFn registerRuntime = nullptr;
		PluginShutdownRuntimeFn shutdownRuntime = nullptr;
		PluginRegisterEditorFn registerEditor = nullptr;
		PluginShutdownEditorFn shutdownEditor = nullptr;
	};

	using PluginEntryFn = const PluginDescriptor* (*)();

	inline constexpr uint32_t PluginApiVersion = 2;
	inline constexpr std::string_view PluginEntryPointName = "CP_GetPluginDescriptor";
}

#if defined(CP_PLATFORM_WINDOWS)
	#define CP_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
	#define CP_PLUGIN_EXPORT extern "C"
#endif

#define CP_DECLARE_PLUGIN(_name, _registerRuntimeFn, _shutdownRuntimeFn, _registerEditorFn, _shutdownEditorFn) \
	CP_PLUGIN_EXPORT const cp::PluginDescriptor* CP_GetPluginDescriptor()               \
	{                                                                                   \
		static const cp::PluginDescriptor descriptor{                                   \
			cp::PluginApiVersion,                                                       \
			_name,                                                                      \
			_registerRuntimeFn,                                                         \
			_shutdownRuntimeFn,                                                         \
			_registerEditorFn,                                                          \
			_shutdownEditorFn                                                           \
		};                                                                              \
		return &descriptor;                                                             \
	}

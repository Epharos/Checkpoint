#pragma once

#include <string_view>

#include "../Log/ILogger.hpp"
#include "../Core/Registry.hpp"

#include "../../Public/Common/Core/Macros.hpp"

namespace cp
{
	class RegistryManager;

	struct PluginHostContext
	{
		ILogger* mainLogger = nullptr;
		RegistryManager* registryManager = nullptr;
	};

	using PluginRegisterFn = bool(*)(PluginHostContext& _context);
	using PluginShutdownFn = void(*)(PluginHostContext& _context);

	struct PluginDescriptor
	{
		uint32_t apiVersion = 0;
		const char* name = nullptr;
		PluginRegisterFn registerPlugin = nullptr;
		PluginShutdownFn shutdownPlugin = nullptr;
	};

	using PluginEntryFn = const PluginDescriptor* (*)();

	inline constexpr uint32_t PluginApiVersion = 1;
	inline constexpr std::string_view PluginEntryPointName = "CP_GetPluginDescriptor";
}

#if defined(CP_PLATFORM_WINDOWS)
	#define CP_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
	#define CP_PLUGIN_EXPORT extern "C"
#endif

#define CP_DECLARE_PLUGIN(_name, _registerFn, _shutdownFn)                              \
	CP_PLUGIN_EXPORT const cp::PluginDescriptor* CP_GetPluginDescriptor()               \
	{                                                                                   \
		static const cp::PluginDescriptor descriptor{                                   \
			cp::PluginApiVersion,                                                       \
			_name,                                                                      \
			_registerFn,                                                                \
			_shutdownFn                                                                 \
		};                                                                              \
		return &descriptor;                                                             \
	}

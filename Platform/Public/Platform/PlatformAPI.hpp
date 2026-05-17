#pragma once

#include <cstdint>
#include <string_view>

#include <IWindow.hpp>
#include <Common/Core/Macros.hpp>

namespace cp
{
	inline constexpr uint32_t PlatformApiVersion = 1;
	inline constexpr std::string_view PlatformEntryPointName = "CP_GetPlatformDescriptor";

	using PlatformCreateWindowFn = IWindow*(*)(const WindowInfo&);
	using PlatformDestroyWindowFn = void(*)(IWindow*);

	struct PlatformDescriptor
	{
		uint32_t apiVersion = 0;
		const char* name = nullptr;
		PlatformCreateWindowFn createWindow = nullptr;
		PlatformDestroyWindowFn destroyWindow = nullptr;
	};

	using PlatformEntryFn = const PlatformDescriptor*(*)();
}

#if defined(CP_PLATFORM_WINDOWS)
	#define CP_PLATFORM_EXPORT extern "C" __declspec(dllexport)
#else
	#define CP_PLATFORM_EXPORT extern "C"
#endif

#define CP_DECLARE_PLATFORM(_name, _createWindowFn, _destroyWindowFn)             \
	CP_PLATFORM_EXPORT const cp::PlatformDescriptor* CP_GetPlatformDescriptor()   \
	{                                                                             \
		static const cp::PlatformDescriptor descriptor {                          \
			cp::PlatformApiVersion,                                               \
			_name,                                                                \
			_createWindowFn,                                                      \
			_destroyWindowFn                                                      \
		};                                                                        \
		return &descriptor;                                                       \
	}

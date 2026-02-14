#pragma once

#include <string_view>
#include <chrono>
#include <thread>

#include "../../Public/Macros.hpp"

#if defined(CP_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

namespace cp
{
	struct ProfilerEvent
	{
		std::string_view name;

		std::chrono::high_resolution_clock::time_point startTime;
		std::chrono::high_resolution_clock::time_point endTime;

		std::thread::id threadId;

#if defined(CP_PLATFORM_WINDOWS)
		ULONG64 startCycleCount;
		ULONG64 endCycleCount;
#endif
	};
}
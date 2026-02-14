#include "ScopedProfiler.hpp"

#include "ProfilerTracker.hpp"

#if defined(CP_PLATFORM_WINDOWS)
	#include <Windows.h>
#endif

namespace cp
{
	ScopedProfiler::ScopedProfiler(std::string_view _name)
	{
		event.name = _name;
		event.startTime = std::chrono::high_resolution_clock::now();
		event.threadId = std::this_thread::get_id();

#if defined(CP_PLATFORM_WINDOWS)
		QueryThreadCycleTime(GetCurrentThread(), &event.startCycleCount);
#endif
	}

	ScopedProfiler::~ScopedProfiler()
	{
		event.endTime = std::chrono::high_resolution_clock::now();

#if defined(CP_PLATFORM_WINDOWS)
		QueryThreadCycleTime(GetCurrentThread(), &event.endCycleCount);
#endif

		ProfilerTracker::GetInstance().RecordEvent(event);
	}
}
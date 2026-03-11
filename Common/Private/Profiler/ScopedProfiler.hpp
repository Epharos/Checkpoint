#pragma once

#include <string_view>

#include "ProfilerEvent.hpp"

#if defined(CP_PROFILING_ENABLED)
#define CP_PROFILE_SCOPE_NAMED(name) cp::ScopedProfiler profiler##__LINE__(name)
#define CP_PROFILE_SCOPE CP_PROFILE_SCOPE_NAMED(__func__)
#else
#define CP_PROFILE_SCOPE_NAMED(name)
#define CP_PROFILE_SCOPE
#endif

namespace cp
{
	class ScopedProfiler
	{
	public:
		/**
		* @brief Starts a profiling scope with the given name. The scope will end when the object goes out of scope.
		* 
		* @param _name The name of the profiling scope, used for identification in the profiler output.
		*/
		ScopedProfiler(std::string_view _name);

		~ScopedProfiler();

	private:
		ProfilerEvent event;
	};
};
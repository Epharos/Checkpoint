#pragma once

#include <string_view>

#include "ProfilerEvent.hpp"

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
#include "ProfilerTracker.hpp"

#include <fstream>

namespace cp
{
	ProfilerTracker& ProfilerTracker::GetInstance()
	{
		static ProfilerTracker instance;
		return instance;
	}

	void ProfilerTracker::RecordEvent(const ProfilerEvent& _event)
	{
		events.push_back(_event);
	}

	void ProfilerTracker::ClearEvents()
	{
		events.clear();
	}

	bool ProfilerTracker::SerializeEvents(const std::string_view& _filename) const
	{
		std::ofstream file(_filename.data());

		if (!file.is_open())
		{
			return false;
		}

		file << "name,thread,start,end,startcycle,endcycle" << std::endl;

		// Serialize as CSV for simplicity, but you can choose any format you prefer (e.g., JSON, XML)
		for (const auto& event : events)
		{
			file << event.name << "," << event.threadId << ","
				<< std::chrono::duration_cast<std::chrono::microseconds>(event.startTime.time_since_epoch()).count() << ","
				<< std::chrono::duration_cast<std::chrono::microseconds>(event.endTime.time_since_epoch()).count()
#if defined(CP_PLATFORM_WINDOWS)
				<< "," << event.startCycleCount
				<< "," << event.endCycleCount
#endif
				<< std::endl;
		}

		file.close();

		return true;
	}
}
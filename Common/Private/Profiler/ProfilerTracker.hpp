#pragma once

#include <string_view>
#include <vector>

#include "ProfilerEvent.hpp"

namespace cp
{
	class ProfilerTracker
	{
	public:

		/**
		* @brief Retrieves the singleton instance of the ProfilerTracker.
		* 
		* @return Reference to the singleton instance of the ProfilerTracker.
		*/
		static ProfilerTracker& GetInstance();

		/**
		* @brief Records a profiling event.
		* 
		* @param _event The profiling event to record.
		*/
		void RecordEvent(const ProfilerEvent& _event);

		/**
		* @brief Clears all recorded profiling events.
		*/
		void ClearEvents();

		/**
		* @brief Serializes all recorded profiling events to a file.
		* 
		* @param _filename The name of the file to which the events will be serialized.
		* 
		* @return True if the events were successfully serialized, false otherwise.
		*/
		bool SerializeEvents(const std::string_view& _filename) const;

	private:
		ProfilerTracker() = default;
		~ProfilerTracker() = default;
		ProfilerTracker(const ProfilerTracker&) = delete;
		ProfilerTracker& operator=(const ProfilerTracker&) = delete;
		ProfilerTracker(ProfilerTracker&&) = delete;
		ProfilerTracker& operator=(ProfilerTracker&&) = delete;

		std::vector<ProfilerEvent> events;
	};
}
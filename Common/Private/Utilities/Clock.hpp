#pragma once

#include <chrono>

namespace cp
{
	class Clock
	{
	private:
		std::chrono::steady_clock::time_point start;

	public:
		/**
		* @brief Constructs a new Clock object and initializes the start time to the current time.
		**/
		Clock();

		/**
		* @brief Resets the start time to the current time and returns the elapsed time since the last reset.
		* @return The elapsed time in seconds as a double.
		**/
		double Restart();

		/**
		* @brief Returns the elapsed time since the last reset without modifying the start time.
		* @return The elapsed time in seconds as a double.
		* @nodiscard
		**/
		[[nodiscard]] 
		double Elapsed() const;
	};
}
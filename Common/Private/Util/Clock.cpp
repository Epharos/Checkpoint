#include "Clock.hpp"

namespace cp
{
	Clock::Clock() : start(std::chrono::steady_clock::now())
	{

	}

	double Clock::Restart()
	{
		auto now = std::chrono::steady_clock::now();
		auto duration = now - start;
		start = now;
		return std::chrono::duration<double>(duration).count();
	}

	[[nodiscard]]
	double Clock::Elapsed() const
	{
		auto now = std::chrono::steady_clock::now();
		auto duration = now - start;
		return std::chrono::duration<double>(duration).count();
	}
}
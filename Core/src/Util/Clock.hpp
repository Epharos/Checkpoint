#pragma once

#include <chrono>

namespace Util
{
	class Clock
	{
	private:
		std::chrono::steady_clock::time_point m_time;

	public:
		Clock()
			: m_time(std::chrono::steady_clock::now())
		{

		}

		double Restart()
		{
			auto now = std::chrono::steady_clock::now();
			auto duration = now - m_time;
			m_time = now;
			return std::chrono::duration<double>(duration).count();
		}

		double Elapsed() const
		{
			auto now = std::chrono::steady_clock::now();
			auto duration = now - m_time;
			return std::chrono::duration<double>(duration).count();
		}
	};
}
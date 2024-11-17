#pragma once

#define CONSOLE_LOG

#define LOG_TRACE(message) Util::Logger::GetInstance().Log(Util::LogLevel::TRACE, message, __FILE__)
#define LOG_DEBUG(message) Util::Logger::GetInstance().Log(Util::LogLevel::DEBUG, message, __FILE__)
#define LOG_INFO(message) Util::Logger::GetInstance().Log(Util::LogLevel::INFO, message, __FILE__)
#define LOG_WARNING(message) Util::Logger::GetInstance().Log(Util::LogLevel::WARNING, message, __FILE__)
#define LOG_ERROR(message) Util::Logger::GetInstance().Log(Util::LogLevel::ERROR, message, __FILE__)
#define LOG_FATAL(message) Util::Logger::GetInstance().Log(Util::LogLevel::FATAL, message, __FILE__)

#include "pch.hpp"

namespace Util
{
	enum LogLevel
	{
		TRACE,
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		FATAL
	};

	class Logger
	{
	public:
		static Logger& GetInstance()
		{
			static Logger instance;
			return instance;
		}

		void Log(LogLevel _level, const std::string& _message, const std::string& _file)
		{
			std::lock_guard<std::mutex> lock(logMutex);

			if (logFile.is_open())
			{
				logFile << FormatLog(_level, _message, _file);
			}

#ifdef CONSOLE_LOG
			std::cout << FormatLog(_level, _message, _file);
#endif
		}

	private:
		std::ofstream logFile;
		std::mutex logMutex;

		Logger()
		{
			if (std::filesystem::create_directory("Logs"))
			{
				std::cout << "Created initial log directory" << std::endl;
			}

			std::stringstream ss;
			ss << "Logs\\" << std::put_time(GetTimestamp(), "%Y-%m-%d%H-%M-%S") << ".log";
			logFile.open(ss.str(), std::ios::app | std::ios::out);

			if (!logFile.is_open())
			{
				std::cerr << "Failed to open log file" << std::endl;
			}
		}

		~Logger()
		{
			if (logFile.is_open())
				logFile.close();
		}

		constexpr std::string_view GetLogLevelString(LogLevel _level)
		{
			switch (_level)
			{
			case LogLevel::TRACE:
				return "TRACE";
			case LogLevel::DEBUG:
				return "DEBUG";
			case LogLevel::INFO:
				return "INFO";
			case LogLevel::WARNING:
				return "WARNING";
			case LogLevel::ERROR:
				return "ERROR";
			case LogLevel::FATAL:
				return "FATAL";
			default:
				return "UNKNOWN";
			}
		}

		std::tm* GetTimestamp()
		{
			auto now = std::chrono::system_clock::now();
			auto nowTime = std::chrono::system_clock::to_time_t(now);
			auto nowTimeTm = new std::tm;
			localtime_s(nowTimeTm, &nowTime);

			return nowTimeTm;
		}

		std::string GetFileName(const std::string& _file)
		{
			auto lastSlash = _file.find_last_of("/\\");
			auto lastDot = _file.find_last_of(".");

			return _file.substr(lastSlash + 1, lastDot - lastSlash - 1);
		}

		std::string FormatLog(LogLevel _level, const std::string& _message, const std::string& _file)
		{
			std::stringstream ss;
			auto timestamp = GetTimestamp();

			ss << std::put_time(timestamp, "%Y/%m/%d %H:%M:%S") << " ";
			ss << "[" << GetLogLevelString(_level) << "] ";
			ss << "[" << GetFileName(_file) << "/" << std::this_thread::get_id() << "] ";
			ss << _message << std::endl;

			return ss.str();
		}
	};
}
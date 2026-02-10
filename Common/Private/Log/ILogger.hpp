#pragma once

#include <string>
#include <string_view>
#include <source_location>
#include <thread>
#include <chrono>

#include "Message.hpp"

#define CP_LOG_EVENT(_logLevel, _message) \
	cp::LogEvent{ \
		.level = _logLevel, \
		.message = _message \
	}

//.timestamp = std::chrono::system_clock::now(), \
//.source = std::source_location::current(), \
//.threadId = std::this_thread::get_id(), \

namespace cp
{
	class IMessageVisitor;

	enum class LogLevel : uint8_t
	{
		Debug,
		Info,
		Warning,
		Error,
		Critical,
	};

	struct LogEvent
	{
		std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
		std::source_location source = std::source_location::current();
		std::thread::id threadId = std::this_thread::get_id();
		LogLevel level;
		Message message;
	};

	class ILogger
	{
	public:
		ILogger(const std::shared_ptr<IMessageVisitor>& _messageVisitor);
		virtual ~ILogger() = default;

		/**
		* @brief Logs a message.
		* 
		* @param _message The message to log.
		* @param _logLevel The log level of the message.
		*/
		virtual void Log(std::string_view _message, LogLevel _logLevel = LogLevel::Info);
		
		/**
		* @brief Logs a structured message.
		* 
		* @param _message The structured message to log.
		* @param _logLevel The log level of the message.
		*/
		virtual void Log(const Message& _message, LogLevel _logLevel = LogLevel::Info);

		/**
		* @brief Logs a log event.
		* 
		* @param _event The log event to log.
		*/
		virtual void Log(const LogEvent& _event);

		/**
		* @brief Inserts vertical spacing in the log output.
		* 
		* @param _verticalSpaces The number of vertical spaces to insert.
		*/
		virtual void Spacing(size_t _verticalSpaces);

		static constexpr const char* LogLevelToString[] = {
			"DEBUG",
			"INFO",
			"WARNING",
			"ERROR",
			"CRITICAL"
		};

	protected:
		std::shared_ptr<IMessageVisitor> messageVisitor;
	};
}
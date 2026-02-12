#pragma once

#include <string>
#include <string_view>
#include <source_location>
#include <thread>
#include <chrono>

#include "Message.hpp"

#define CP_LOG_EVENT(_logLevel, _label, _message) \
	cp::LogEvent{ \
		.level = _logLevel, \
		.label = _label, \
		.message = _message \
	}

namespace cp
{
	class IMessageVisitor;

	struct LogLevel
	{
		std::string name;
		size_t severity;
	};

	struct LogEvent
	{
		std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
		std::source_location source = std::source_location::current();
		std::thread::id threadId = std::this_thread::get_id();
		LogLevel level;
		std::string_view label;
		Message message;
	};

	class ILogger
	{
	public:
		/**
		* @brief Constructs a new ILogger instance with the given message visitor and log level threshold.
		* 
		* @param _messageVisitor The message visitor to use for dispatching log messages.
		* @param _logLevelThreshold The log level threshold for filtering log messages.
		*/
		ILogger(const std::shared_ptr<IMessageVisitor>& _messageVisitor, size_t _logLevelThreshold = 0);

		virtual ~ILogger() = default;

		/**
		* @brief Logs a message.
		* 
		* @param _message The message to log.
		* @param _logLevel The log level of the message.
		*/
		virtual void Log(std::string_view _message, LogLevel _logLevel = ILogger::Info);
		
		/**
		* @brief Logs a structured message.
		* 
		* @param _message The structured message to log.
		* @param _logLevel The log level of the message.
		*/
		virtual void Log(const Message& _message, LogLevel _logLevel = ILogger::Info);

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

	protected:
		/**
		* @brief Determines whether a message with the given log level should be logged based on the current log level threshold.
		* 
		* @param _logLevel The log level of the message to check.
		* 
		* @return true if the message should be logged, false otherwise.
		*/
		virtual bool ShouldLog(LogLevel _logLevel) const;

	public:
		static LogLevel Trace;
		static LogLevel Debug;
		static LogLevel Info;
		static LogLevel Warning;
		static LogLevel Error;
		static LogLevel Critical;

	protected:
		std::shared_ptr<IMessageVisitor> messageVisitor = nullptr;
		size_t logLevelThreshold = 0; // Only log messages with severity >= threshold
	};
}
#pragma once

#include "../ILogger.hpp"

namespace cp
{
	class ConsoleLogger : public ILogger
	{
	public:
		ConsoleLogger(const std::shared_ptr<IMessageVisitor>& _messageVisitor);

		/**
		* @brief Logs a structured message to the console.
		* 
		* @param _message The structured message to log.
		* @param _logLevel The log level of the message.
		*/
		void Log(const Message& _message, LogLevel _logLevel) override;

		/**
		* @brief Logs a log event to the console.
		* 
		* @param _event The log event to log.
		*/
		void Log(const LogEvent& _event) override;

		/**
		* @brief Inserts vertical spacing in the console output.
		* 
		* @param _verticalSpaces The number of vertical spaces to insert.
		*/
		void Spacing(size_t _verticalSpaces) override;
	};
}
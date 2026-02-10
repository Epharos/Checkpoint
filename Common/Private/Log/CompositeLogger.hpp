#pragma once

#include "ILogger.hpp"
#include <vector>
#include <mutex>

namespace cp
{
	class CompositeLogger : public ILogger
	{
	public:
		CompositeLogger();

		/**
		* @brief Adds a logger to the composite.
		*
		* @param _logger The logger to add.
		*/
		void AddLogger(std::shared_ptr<ILogger> _logger);

		/**
		* @brief Logs a message to all contained loggers.
		*
		* @param _message The message to log.
		* @param _logLevel The log level of the message.
		*/
		void Log(std::string_view _message, LogLevel _logLevel) override;

		/**
		* @brief Logs a structured message to all contained loggers.
		* 
		* @param _message The structured message to log.
		* @param _logLevel The log level of the message.
		*/
		void Log(const Message& _message, LogLevel _logLevel) override;

		/**
		* @brief Logs a log event to all contained loggers.
		* 
		* @param _event The log event to log.
		*/
		void Log(const LogEvent& _event) override;

		/**
		* @brief Inserts vertical spacing in all contained loggers.
		*
		* @param _verticalSpaces The number of vertical spaces to insert.
		*/
		void Spacing(size_t _verticalSpaces) override;

	private:
		std::vector<std::shared_ptr<ILogger>> loggers;
		std::mutex mutex;
	};
}
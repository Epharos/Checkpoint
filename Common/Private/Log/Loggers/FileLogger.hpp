#pragma once

#include "../ILogger.hpp"

#include <fstream>
#include <string>

namespace cp
{
	class FileLogger : public ILogger
	{
	public:
		/**
		* @brief Constructs a FileLogger that logs to the specified file.
		*
		* @param _filePath The path to the log file.
		*/
		explicit FileLogger(const std::shared_ptr<IMessageVisitor>& _messageVisitor, const std::string& _filePath);

		/**
		* @brief Logs a structured message to the file.
		* 
		* @param _message The structured message to log.
		* @param _logLevel The log level of the message.
		*/
		void Log(const Message& _message, LogLevel _logLevel) override;

		/**
		* @brief Logs a log event to the file.
		* 
		* @param _event The log event to log.
		*/
		void Log(const LogEvent& _event) override;

		/**
		* @brief Inserts vertical spacing in the file output.
		*
		* @param _verticalSpaces The number of vertical spaces to insert.
		*/
		void Spacing(size_t _verticalSpaces) override;

	private:
		std::ofstream logFile;
	};
}
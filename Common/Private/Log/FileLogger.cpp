#include "FileLogger.hpp"

#include <syncstream>
#include <sstream>

#include "Visitors/IMessageVisitor.hpp"
#include "Message.hpp"

namespace cp
{
	namespace
	{
		void WriteLogHeader(std::stringstream& _ss, const LogEvent& _event)
		{
			auto sourceFile = std::string(_event.source.file_name());
			auto fileSepPos = sourceFile.find_last_of("/\\");
			auto fileNameOnly = sourceFile.substr(fileSepPos + 1);

			auto timepointAsSecond = std::chrono::floor<std::chrono::seconds>(_event.timestamp);

			_ss << std::format("{:%Y/%m/%d %T} ", timepointAsSecond)
				<< _event.label << " "
				<< _event.level.name
				<< " [" << fileNameOnly << ":" << _event.source.line() << "/"
				<< _event.threadId << "] - ";
		}
	}

	FileLogger::FileLogger(const std::shared_ptr<IMessageVisitor>& _messageVisitor, const std::string& _filePath)
		: ILogger(_messageVisitor), logFile(_filePath, std::ios::app)
	{
		if (!logFile.is_open())
		{
			throw std::runtime_error("Failed to open log file: " + _filePath);
		}
	}

	void FileLogger::Log(const Message& _message, LogLevel _logLevel)
	{
		std::stringstream ss;

		for (auto& component : _message.components)
		{
			messageVisitor->Dispatch(*component, &ss);
		}

		std::osyncstream(logFile) << ss.str() << std::endl;
	}

	void FileLogger::Log(const LogEvent& _event)
	{
		std::stringstream ss;

		WriteLogHeader(ss, _event);

		for (auto& component : _event.message.components)
		{
			messageVisitor->Dispatch(*component, &ss);
		}

		std::osyncstream(logFile) << ss.str() << std::endl;
	}

	void FileLogger::Spacing(size_t _verticalSpaces)
	{
		std::stringstream ss;

		for (size_t i = 0; i < _verticalSpaces; i++)
		{
			ss << std::endl;
		}

		std::osyncstream(logFile) << ss.str();
	}
}
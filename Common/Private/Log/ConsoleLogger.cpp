#include "ConsoleLogger.hpp"

#include <iostream>
#include <syncstream>
#include <sstream>

#include "Visitors/IMessageVisitor.hpp"
#include "Message.hpp"

#include "../Utilities/TerminalFormattingHelper.hpp"

namespace cp
{
	namespace
	{
		cp::TerminalColor GetLogLevelColor(LogLevel _logLevel)
		{
			if (_logLevel.severity < ILogger::Debug.severity)
				return TerminalColor::Cyan;

			if (_logLevel.severity < ILogger::Info.severity)
				return TerminalColor::Blue;

			if (_logLevel.severity < ILogger::Warning.severity)
				return TerminalColor::Green;

			if (_logLevel.severity >= ILogger::Critical.severity)
				return TerminalColor::Magenta;

			if (_logLevel.severity >= ILogger::Error.severity)
				return TerminalColor::Red;

			return TerminalColor::Yellow;
		}

		void WriteLogHeader(std::stringstream& _ss, const LogEvent& _event)
		{
			auto sourceFile = std::string(_event.source.file_name());
			auto fileSepPos = sourceFile.find_last_of("/\\");
			auto fileNameOnly = sourceFile.substr(fileSepPos + 1);

			auto timepointAsSecond = std::chrono::floor<std::chrono::seconds>(_event.timestamp);

			_ss << std::format("{:%Y/%m/%d %T} ", timepointAsSecond)
				<< GetFormattingCode(TerminalColor::Black, TerminalColor::White) << _event.label << GetTerminalResetCode() << " "
				<< GetFormattingCode(TerminalColor::Black, GetLogLevelColor(_event.level)) << _event.level.name << GetTerminalResetCode()
				<< " [" << GetTerminalForegroundColorCode(TerminalColor::Cyan) << fileNameOnly << GetTerminalResetCode() << ":" << _event.source.line() << "/"
				<< GetTerminalForegroundColorCode(TerminalColor::Yellow) << _event.threadId << GetTerminalResetCode() << "] - ";
		}
	}

	ConsoleLogger::ConsoleLogger(const std::shared_ptr<IMessageVisitor>& _messageVisitor)
		: ILogger(_messageVisitor)
	{

	}

	void ConsoleLogger::Log(const Message& _message, LogLevel _logLevel)
	{
		std::stringstream ss;

		for (auto& component : _message.components)
		{
			messageVisitor->Dispatch(*component, &ss);
		}

		std::osyncstream(std::cout) << ss.str() << std::endl;
	}

	void ConsoleLogger::Log(const LogEvent& _event)
	{
		std::stringstream ss;

		WriteLogHeader(ss, _event);

		for (auto& component : _event.message.components)
		{
			messageVisitor->Dispatch(*component, &ss);
		}

		std::osyncstream(std::cout) << ss.str() << std::endl;
	}

	void ConsoleLogger::Spacing(size_t _verticalSpaces)
	{
		std::stringstream ss;

		for (size_t i = 0; i < _verticalSpaces; i++)
		{
			ss << std::endl;
		}

		std::osyncstream(std::cout) << ss.str();
	}
}
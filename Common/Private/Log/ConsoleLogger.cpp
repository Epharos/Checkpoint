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
			switch (_logLevel)
			{
			case LogLevel::Debug:
				return TerminalColor::Blue;
			case LogLevel::Info:
				return TerminalColor::Green;
			case LogLevel::Warning:
				return TerminalColor::Yellow;
			case LogLevel::Error:
				return TerminalColor::Red;
			case LogLevel::Critical:
				return TerminalColor::Magenta;
			default:
				return TerminalColor::Default;
			}
		}

		void WriteLogHeader(std::stringstream& _ss, const LogEvent& _event)
		{
			auto sourceFile = std::string(_event.source.file_name());
			auto fileSepPos = sourceFile.find_last_of("/\\");
			auto fileNameOnly = sourceFile.substr(fileSepPos + 1);

			auto timepointAsSecond = std::chrono::floor<std::chrono::seconds>(_event.timestamp);

			_ss << std::format("{:%Y/%m/%d %T} ", timepointAsSecond)
				<< GetFormattingCode(TerminalColor::Black, GetLogLevelColor(_event.level)) << ILogger::LogLevelToString[static_cast<uint8_t>(_event.level)] << GetTerminalResetCode()
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
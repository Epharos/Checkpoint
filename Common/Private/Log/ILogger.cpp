#include "ILogger.hpp"

#include <format>
#include <chrono>

#include "Message.hpp"
#include "Visitors/IMessageVisitor.hpp"
#include "Components/TextComponent.hpp"

namespace cp
{
	LogLevel ILogger::Trace = LogLevel{ "Trace", 0 };
	LogLevel ILogger::Debug = LogLevel{ "Debug", 100 };
	LogLevel ILogger::Info = LogLevel{ "Info", 200 };
	LogLevel ILogger::Warning = LogLevel{ "Warning", 300 };
	LogLevel ILogger::Error = LogLevel{ "Error", 400 };
	LogLevel ILogger::Critical = LogLevel{ "Critical", 500 };

	ILogger::ILogger(const std::shared_ptr<IMessageVisitor>& _messageVisitor, size_t _logLevelThreshold) 
		: messageVisitor(_messageVisitor), logLevelThreshold(_logLevelThreshold)
	{

	}

	void ILogger::Log(std::string_view _message, LogLevel _logLevel)
	{
		if (!ShouldLog(_logLevel)) return;

		cp::Message message = cp::Message::Create<cp::TextComponent>(_message);

		for (const auto& component : message.components)
		{
			messageVisitor->Dispatch(*component);
		}
	}

	void ILogger::Log(const Message& _message, LogLevel _logLevel)
	{
		if (!ShouldLog(_logLevel)) return;

		for (const auto& component : _message.components)
		{
			messageVisitor->Dispatch(*component);
		}
	}

	void ILogger::Log(const LogEvent& _event)
	{
		if (!ShouldLog(_event.level)) return;

		for (const auto& component : _event.message.components)
		{
			messageVisitor->Dispatch(*component);
		}
	}

	void ILogger::Spacing(size_t _verticalSpaces)
	{

	}

	bool ILogger::ShouldLog(LogLevel _logLevel) const
	{
		return messageVisitor && _logLevel.severity >= logLevelThreshold;
	}
}

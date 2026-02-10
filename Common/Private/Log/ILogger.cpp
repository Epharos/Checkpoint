#include "ILogger.hpp"

#include <format>
#include <chrono>

#include "Message.hpp"
#include "Visitors/IMessageVisitor.hpp"
#include "Components/TextComponent.hpp"

namespace cp
{
	ILogger::ILogger(const std::shared_ptr<IMessageVisitor>& _messageVisitor) : messageVisitor(_messageVisitor)
	{

	}

	void ILogger::Log(std::string_view _message, LogLevel _logLevel)
	{
		if (!messageVisitor) return;

		cp::Message message = cp::Message::Create<cp::TextComponent>(_message);

		for (const auto& component : message.components)
		{
			messageVisitor->Dispatch(*component);
		}
	}

	void ILogger::Log(const Message& _message, LogLevel _logLevel)
	{
		if (!messageVisitor) return;

		for (const auto& component : _message.components)
		{
			messageVisitor->Dispatch(*component);
		}
	}

	void ILogger::Log(const LogEvent& _event)
	{
		if (!messageVisitor) return;

		for (const auto& component : _event.message.components)
		{
			messageVisitor->Dispatch(*component);
		}
	}

	void ILogger::Spacing(size_t _verticalSpaces)
	{

	}
}

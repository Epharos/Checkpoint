#include "SurroundedComponent.hpp"

#include "../Message.hpp"

#include "../../Assert.hpp"

namespace cp
{
	SurroundedComponent::SurroundedComponent(std::string_view _prefix, std::string_view _suffix, const Message& _message)
		: prefix(_prefix), suffix(_suffix), message(_message)
	{
		CP_EXPECT_MSG(!prefix.empty() && !suffix.empty(), "Prefix and suffix cannot be empty.");
		CP_EXPECT_MSG(&message != nullptr, "Message cannot be null.");
		CP_EXPECT_MSG(message.components.size() > 0, "Message cannot be empty.");
	}

	SurroundedComponent::~SurroundedComponent() = default;

	MessageComponentTypeId SurroundedComponent::GetTypeId() const
	{
		return StaticTypeId();
	}

	MessageComponentTypeId SurroundedComponent::StaticTypeId()
	{
		return GetMessageComponentTypeId<SurroundedComponent>();
	}

	std::string_view SurroundedComponent::GetPrefix() const
	{
		return prefix;
	}

	std::string_view SurroundedComponent::GetSuffix() const
	{
		return suffix;
	}

	Message SurroundedComponent::GetMessage() const
	{
		return message;
	}
}
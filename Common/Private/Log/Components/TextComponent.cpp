#include "TextComponent.hpp"

#include "../Visitors/IMessageVisitor.hpp"

#include "../../Assert.hpp"


namespace cp
{
	TextComponent::~TextComponent() = default;

	MessageComponentTypeId TextComponent::GetTypeId() const
	{
		return StaticTypeId();
	}

	MessageComponentTypeId TextComponent::StaticTypeId()
	{
		return GetMessageComponentTypeId<TextComponent>();
	}

	std::string_view TextComponent::GetText() const
	{
		return text;
	}
}
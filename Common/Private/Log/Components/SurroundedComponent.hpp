#pragma once

#include "IMessageComponent.hpp"

#include <string_view>
#include <string>

#include "../Message.hpp"

namespace cp
{
	class IMessageVisitor;

	/**
	* @brief A message component that represents plain text.
	*/
	class SurroundedComponent : public IMessageComponent
	{
	public:
		/**
		* @brief Constructs a SurroundedComponent with the given parameters.
		*
		* @param _prefix The prefix to surround the message with.
		* @param _suffix The suffix to surround the message with.
		* @param _message The message to surround with the prefix and suffix.
		*/
		explicit SurroundedComponent(std::string_view _prefix, std::string_view _suffix, const Message& _message);

		~SurroundedComponent() override;

		MessageComponentTypeId GetTypeId() const override;

		static MessageComponentTypeId StaticTypeId();

		/**
		* @brief Gets the prefix of the message.
		*
		* @return The text of the messagez.
		*/
		std::string_view GetPrefix() const;

		/**
		* @brief Gets the suffix of the message.
		* 
		* @return The suffix of the message.
		*/
		std::string_view GetSuffix() const;

		/**
		* @brief Gets the message contained in this component.
		* 
		* @return The message contained in this component.
		*/
		Message GetMessage() const;

	private:
		std::string prefix;
		std::string suffix;
		Message message;
	};
}
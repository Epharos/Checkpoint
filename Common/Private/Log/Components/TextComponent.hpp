#pragma once

#include "IMessageComponent.hpp"

#include <string_view>
#include <string>
#include <format>

#include "../../Core/Assert.hpp"

namespace cp
{
	class Message;
	class IMessageVisitor;

	/**
	* @brief A message component that represents plain text.
	*/
	class TextComponent : public IMessageComponent
	{
	public:
		/**
		* @brief Constructs a TextComponent with the given text.
		* 
		* @param _text The text of the component.
		*/
		/*explicit TextComponent(std::string_view _text)
		{
			CP_EXPECT_MSG(!_text.empty(), "TextComponent cannot be empty");
			text = _text;
		}*/

		/**
		* @brief Constructs a TextComponent with the given text.
		* 
		* @param _format The format string of the component.
		* @param args The arguments to format the string with.
		*/
		template<typename... Args>
		explicit TextComponent(std::string_view _format, Args&&... args)
		{
			CP_EXPECT_MSG(!_format.empty(), "TextComponent format string cannot be null");
			text = std::vformat(_format, std::make_format_args(std::forward<Args>(args)...));
		}

		~TextComponent() override;

		MessageComponentTypeId GetTypeId() const override;

		static MessageComponentTypeId StaticTypeId();

		/**
		* @brief Gets the text of the component.
		* 
		* @return The text of the component.
		*/
		std::string_view GetText() const;

	private:
		std::string text;

	};
}
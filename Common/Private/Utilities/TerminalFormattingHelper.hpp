#pragma once

#include <string>
#include <vector>

namespace cp
{
	enum class TerminalColor
	{
		Default,
		Black,
		Red,
		Green,
		Yellow,
		Blue,
		Magenta,
		Cyan,
		White,

		Unchanged = -1
	};

	enum class TerminalStyle
	{
		Default,
		Bold,
		Dim,
		Italic,
		Underline,
		Blink,
		Inverted,
		Hidden,
		Strikethrough,

		Unchanged = -1
	};

	/**
	* @brief Gets the ANSI escape code for the specified terminal foreground color.
	* 
	* @param color The foreground terminal color to get the escape code for.
	* 
	* @return The ANSI escape code for the specified terminal foreground color.
	*/
	std::string_view GetTerminalForegroundColorCode(TerminalColor color);

	/**
	* @brief Gets the ANSI escape code for the specified terminal background color.
	* 
	* @param color The background terminal color to get the escape code for.
	* 
	* @return The ANSI escape code for the specified terminal background color.
	*/
	std::string_view GetTerminalBackgroundColorCode(TerminalColor color);

	/**
	* @brief Gets the ANSI escape code for the specified terminal style.
	* 
	* @param style The terminal style to get the escape code for.
	* 
	* @return The ANSI escape code for the specified terminal style.
	*/
	std::string GetTerminalStyleCode(TerminalStyle style);

	/**
	* @brief Gets the ANSI escape code to reset all terminal formatting.
	* 
	* @return The ANSI escape code to reset all terminal formatting.
	*/
	std::string_view GetTerminalResetCode();
	
	/**
	* @brief Gets the ANSI escape code for the specified terminal formatting options.
	* 
	* @param _foreground The foreground terminal color.
	* @param _background The background terminal color (default is TerminalColor::Default).
	* @param _styles A vector of terminal styles to apply (default is an empty vector).
	* 
	* @return The ANSI escape code for the specified terminal formatting options.
	*/
	std::string GetFormattingCode(TerminalColor _foreground, TerminalColor _background = TerminalColor::Default, std::vector<TerminalStyle> _styles = {});
}
#include "TerminalFormattingHelper.hpp"

#include <stdexcept>

std::string_view cp::GetTerminalForegroundColorCode(TerminalColor color)
{
	static std::string_view codes[] = {
		"\033[39m", // Default
		"\033[30m", // Black
		"\033[31m", // Red
		"\033[32m", // Green
		"\033[33m", // Yellow
		"\033[34m", // Blue
		"\033[35m", // Magenta
		"\033[36m", // Cyan
		"\033[37m", // White
		""			// Unchanged
	};

	return codes[static_cast<int>(color)];
}

std::string_view cp::GetTerminalBackgroundColorCode(TerminalColor color)
{
	static std::string_view codes[] = {
		"\033[49m", // Default
		"\033[40m", // Black
		"\033[41m", // Red
		"\033[42m", // Green
		"\033[43m", // Yellow
		"\033[44m", // Blue
		"\033[45m", // Magenta
		"\033[46m", // Cyan
		"\033[47m", // White
		""			// Unchanged
	};

	return codes[static_cast<int>(color)];
}

std::string cp::GetTerminalStyleCode(TerminalStyle style)
{
	throw std::runtime_error("GetTerminalStyleCode is not implemented yet.");
}

std::string_view cp::GetTerminalResetCode()
{
	static std::string_view code = "\033[0m";
	return code;
}

std::string cp::GetFormattingCode(TerminalColor _foreground, TerminalColor _background, std::vector<TerminalStyle> _styles)
{
	std::string code;

	code += GetTerminalForegroundColorCode(_foreground);
	code += GetTerminalBackgroundColorCode(_background);

	for (const auto& style : _styles)
	{
		code += GetTerminalStyleCode(style);
	}

	return code;
}

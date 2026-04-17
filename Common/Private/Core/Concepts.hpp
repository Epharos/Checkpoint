#pragma once

#include <type_traits>

namespace cp
{
	template<typename T>
	concept Numeric = std::integral<T> || std::floating_point<T>;

	template<typename T>
	concept Enum = std::is_enum_v<T>;
}
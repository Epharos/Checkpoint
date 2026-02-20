#pragma once

namespace cp
{
	template<typename T>
	concept Numeric = std::integral<T> || std::floating_point<T>;
}
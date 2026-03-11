#pragma once

#include <../../../Common/Public/Common/Core/Concepts.hpp>

namespace cp
{
	// I mean, while we wait for C++26's reflection ...

	template<Enum Dst, Enum Src>
	constexpr Dst EnumCast(Src value)
	{
		throw std::logic_error("Default EnumCast called. It indicates a bug in the application.");
		return static_cast<Dst>(value);
	}

	template<typename Dst, Enum Src>
	constexpr Dst EnumBitsCast(Src value)
	{
		throw std::logic_error("Default EnumBitsCast called. It indicates a bug in the application.");
		return static_cast<Dst>(0);
	}

	template<typename Dst, Enum DstBits, Enum Src, size_t EnumSize>
	constexpr Dst ConvertBitField(Src value)
	{
		Dst result = static_cast<Dst>(0);

		for (size_t i = 0; i < EnumSize; ++i)
		{
			if (static_cast<uint32_t>(value) & (1 << i))
			{
				result |= EnumCast<DstBits, Src>(static_cast<Src>(1 << i));
			}
		}

		return result;
	}
}
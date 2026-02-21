#pragma once

#include <cstdint>

namespace cp
{
	enum class Format : uint32_t
	{
		Unknown,

		R8G8B8A8_UNORM,
		B8G8R8A8_UNORM,

		R16G16B16A16_FLOAT,

		R32_UINT,

		D24_UNORM_S8_UINT,
		D32_FLOAT,

		Count
	};
}
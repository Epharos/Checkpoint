#pragma once

#include <cstdint>

namespace cp
{
	/**
	 * @brief Shader stages visibility mask.
	 *        This is intended to be backend-agnostic (Vulkan/DX12/Metal).
	 */
	enum class ShaderStage : uint32_t
	{
		None = 0,

		Vertex = 1u << 0,

		Geometry = 1u << 1,

		TessellationControl = 1u << 2,
		TessellationEvaluation = 1u << 3,

		Fragment = 1u << 4,
		Pixel = Fragment,

		Compute = 1u << 5,

		Mesh = 1u << 6,
		Task = 1u << 7,

		AllGraphics = Vertex | Geometry | TessellationControl | TessellationEvaluation | Fragment,
		All = AllGraphics | Mesh | Task | Compute,
	};

	constexpr ShaderStage operator|(ShaderStage _a, ShaderStage _b)
	{
		return static_cast<ShaderStage>(static_cast<uint32_t>(_a) | static_cast<uint32_t>(_b));
	}

	constexpr ShaderStage operator&(ShaderStage _a, ShaderStage _b)
	{
		return static_cast<ShaderStage>(static_cast<uint32_t>(_a) & static_cast<uint32_t>(_b));
	}

	constexpr ShaderStage& operator|=(ShaderStage& _a, const ShaderStage _b)
	{
		_a = _a | _b;
		return _a;
	}

	constexpr bool Any(ShaderStage _stages)
	{
		return static_cast<uint32_t>(_stages) != 0;
	}
}

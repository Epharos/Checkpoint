#pragma once

#include <cstdint>
#include <vector>

#include "../Data/Enums.hpp"

namespace cp
{
	// -------------------------
	// Vertex Input
	// -------------------------

	enum class VertexInputRate : uint8_t
	{
		PerVertex,
		PerInstance
	};

	/**
	 * @brief Vertex attribute format.
	 *		  Separate from Format used for Textures and such.
	 */
	enum class VertexFormat : uint16_t
	{
		Unknown,

		R32_FLOAT,
		R32G32_FLOAT,
		R32G32B32_FLOAT,
		R32G32B32A32_FLOAT,

		R8G8B8A8_UNORM,
	};

	struct VertexBindingInfo
	{
		uint32_t binding = 0;
		uint32_t strideBytes = 0;
		VertexInputRate inputRate = VertexInputRate::PerVertex;
	};

	struct VertexAttributeInfo
	{
		uint32_t location = 0;
		uint32_t binding = 0;
		VertexFormat format = VertexFormat::Unknown;
		uint32_t offsetBytes = 0;
	};

	struct VertexInputState
	{
		std::vector<VertexBindingInfo> bindings;
		std::vector<VertexAttributeInfo> attributes;
	};

	// -------------------------
	// Input Assembly / Raster
	// -------------------------

	enum class PrimitiveTopology : uint8_t
	{
		TriangleList,
		TriangleStrip,
		LineList,
		LineStrip,
		PointList
	};

	enum class PolygonMode : uint8_t
	{
		Fill,
		Line,
		Point
	};

	enum class CullMode : uint8_t
	{
		None,
		Front,
		Back
	};

	enum class FrontFace : uint8_t
	{
		CounterClockwise,
		Clockwise
	};

	struct RasterizationState
	{
		PolygonMode polygonMode = PolygonMode::Fill;
		CullMode cullMode = CullMode::Back;
		FrontFace frontFace = FrontFace::CounterClockwise;

		bool depthClampEnable = false;

		bool depthBiasEnable = false;
		float depthBiasConstantFactor = 0.0f;
		float depthBiasSlopeFactor = 0.0f;
	};

	// -------------------------
	// Depth / Stencil
	// -------------------------

	enum class CompareOp : uint8_t
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	struct DepthStencilState
	{
		bool depthTestEnable = true;
		bool depthWriteEnable = true;
		CompareOp depthCompareOp = CompareOp::LessOrEqual;

		bool stencilTestEnable = false;
	};

	// -------------------------
	// Blending
	// -------------------------

	enum class BlendFactor : uint8_t
	{
		Zero,
		One,

		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,

		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha
	};

	enum class BlendOp : uint8_t
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class ColorWriteMask : uint8_t
	{
		None = 0,
		R = 1u << 0,
		G = 1u << 1,
		B = 1u << 2,
		A = 1u << 3,
		All = 0xFu
	};

	constexpr ColorWriteMask operator|(ColorWriteMask _a, ColorWriteMask _b)
	{
		return static_cast<ColorWriteMask>(static_cast<uint8_t>(_a) | static_cast<uint8_t>(_b));
	}

	constexpr ColorWriteMask& operator|=(ColorWriteMask& _a, ColorWriteMask _b)
	{
		_a = _a | _b;
		return _a;
	}

	struct BlendAttachmentState
	{
		bool blendEnable = false;

		BlendFactor srcColorFactor = BlendFactor::One;
		BlendFactor dstColorFactor = BlendFactor::Zero;
		BlendOp colorOp = BlendOp::Add;

		BlendFactor srcAlphaFactor = BlendFactor::One;
		BlendFactor dstAlphaFactor = BlendFactor::Zero;
		BlendOp alphaOp = BlendOp::Add;

		ColorWriteMask writeMask = ColorWriteMask::All;
	};
}

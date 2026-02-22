#pragma once

#include <cstdint>

#include <Extent.hpp>
#include <Formats.hpp>

namespace cp
{
	enum class TextureUsage : uint32_t
	{
		Sampled = 1 << 0,
		ColorAttachment = 1 << 1,
		DepthStencilAttachment = 1 << 2,
		Storage = 1 << 3,
		// In case more usages are added, keep in mind that the converters will need to be updated to handle the new flags.
	};

	enum class TextureAspect : uint32_t
	{
		None = 0,
		Color = 1 << 1,
		Depth = 1 << 2,
		Stencil = 1 << 3,

		// In case more usages are added, keep in mind that the converters will need to be updated to handle the new flags.

		// Common combinations

		DepthStencil = Depth | Stencil
	};

	enum class TextureType
	{
		Texture2D,
		Texture3D,
		Texture2DArray,

		Count,
	};

	struct TextureInfo
	{
		TextureType type;

		Extent3D<uint32_t> extent;

		uint32_t mipLevels;
		uint32_t arrayLayers;

		Format format;
		TextureUsage usage;
		TextureAspect aspect;
	};

	class ITexture
	{
	public:
		ITexture(const TextureInfo& _info);
		virtual ~ITexture() = default;

	protected:
		TextureInfo info;
	};
}
#pragma once

#include <cstdint>

#include <Common/Data/Extent.hpp>

#include "Enums.hpp"

namespace cp
{
	enum class TextureUsage : uint32_t
	{
		Sampled = 1 << 0,
		ColorAttachment = 1 << 1,
		DepthStencilAttachment = 1 << 2,
		Storage = 1 << 3,

		TransferSrc = 1 << 4,
		TransferDst = 1 << 5,
	};

	constexpr TextureUsage operator|(TextureUsage _lhs, TextureUsage _rhs)
	{
		return static_cast<TextureUsage>(static_cast<uint32_t>(_lhs) | static_cast<uint32_t>(_rhs));
	}

	constexpr TextureUsage& operator|=(TextureUsage& _lhs, const TextureUsage _rhs)
	{
		_lhs = _lhs | _rhs;
		return _lhs;
	}

	enum class TextureAspect : uint32_t
	{
		None = 0,
		Color = 1 << 1,
		Depth = 1 << 2,
		Stencil = 1 << 3,

		DepthStencil = Depth | Stencil
	};

	constexpr TextureAspect operator|(TextureAspect _lhs, TextureAspect _rhs)
	{
		return static_cast<TextureAspect>(static_cast<uint32_t>(_lhs) | static_cast<uint32_t>(_rhs));
	}

	constexpr TextureAspect& operator|=(TextureAspect& _lhs, const TextureAspect _rhs)
	{
		_lhs = _lhs | _rhs;
		return _lhs;
	}

	enum class TextureLayout : uint32_t
	{
		Undefined,
		General,

		AttachmentOptimal,
		ColorAttachment,
		DepthStencilAttachment,
		DepthStencilReadOnly,

		ShaderReadOnly,

		TransferSrc,
		TransferDst,

		ResolveSrc,
		ResolveDst,

		Present
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
		explicit ITexture(const TextureInfo& _info);
		virtual ~ITexture() = default;

		/**
		* @brief Returns the current layout of this texture.
		*        Reflects the last layout transition performed via a barrier.
		*
		* @return The current texture layout.
		*/
		[[nodiscard]] TextureLayout GetLayout() const { return layout; }

		/**
		* @brief Updates the cached layout of this texture on the CPU side.
		*        Must be called after recording a barrier that transitions this texture to a new layout.
		*
		* @param _layout The new layout of the texture.
		*/
		void UpdateLayout(TextureLayout _layout);

	protected:
		const TextureInfo info;

		TextureLayout layout = TextureLayout::Undefined;
	};
}
#pragma once

#include <cstdint>

namespace cp
{
	enum class Filter : uint8_t
	{
		Nearest,
		Linear
	};

	enum class MipmapMode : uint8_t
	{
		Nearest,
		Linear
	};

	enum class AddressMode : uint8_t
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	struct SamplerInfo
	{
		Filter minFilter = Filter::Linear;
		Filter magFilter = Filter::Linear;
		MipmapMode mipmapMode = MipmapMode::Linear;

		AddressMode addressU = AddressMode::Repeat;
		AddressMode addressV = AddressMode::Repeat;
		AddressMode addressW = AddressMode::Repeat;

		bool anisotropyEnable = false;
		float maxAnisotropy = 1.0f;
	};

	class ISampler
	{
	public:
		explicit ISampler(const SamplerInfo& _samplerInfo) : info(_samplerInfo) {}
		virtual ~ISampler() = default;

	protected:
		const SamplerInfo& info;
	};
}

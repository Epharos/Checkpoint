#pragma once

#include <cstdint>
#include <vector>

#include "ITexture.hpp"

namespace cp
{
	class IDescriptorSetLayout;

	class IBuffer;
	class ISampler;

	struct DescriptorBufferBinding
	{
		uint32_t binding = 0;
		IBuffer* buffer = nullptr;
		uint64_t offsetBytes = 0;
		uint64_t rangeBytes = ~0ull;
	};

	struct DescriptorTextureBinding
	{
		uint32_t binding = 0;
		ITexture* texture = nullptr;
		TextureLayout layout = TextureLayout::Undefined;
		ISampler* sampler = nullptr;
	};

	class IDescriptorSet
	{
	public:
		explicit IDescriptorSet(const IDescriptorSetLayout& _layout) : descriptorSetLayout(_layout) {}
		virtual ~IDescriptorSet() = default;

		virtual void UpdateBuffers(const std::vector<DescriptorBufferBinding>& _buffers) = 0;
		virtual void UpdateTextures(const std::vector<DescriptorTextureBinding>& _textures) = 0;

		[[nodiscard]] virtual const IDescriptorSetLayout& GetLayout() const = 0;

	protected:
		const IDescriptorSetLayout& descriptorSetLayout;
	};
}

#pragma once

#include <cstdint>
#include <vector>

#include "../Rendering/ShaderStage.hpp"

namespace cp
{
	enum class DescriptorType : uint8_t
	{
		UniformBuffer,
		StorageBuffer,

		SampledTexture,
		StorageTexture,

		Sampler,
		CombinedImageSampler
	};

	struct DescriptorBinding
	{
		uint32_t binding = 0;
		DescriptorType type = DescriptorType::UniformBuffer;
		uint32_t count = 1;
		ShaderStage visibility = ShaderStage::All;
	};

	struct DescriptorSetLayoutInfo
	{
		std::vector<DescriptorBinding> bindings;
	};

	class IDescriptorSetLayout
	{
	public:
		explicit IDescriptorSetLayout(const DescriptorSetLayoutInfo& _info) : info(_info) {}
		virtual ~IDescriptorSetLayout() = default;

	protected:
		const DescriptorSetLayoutInfo& info;
	};
}

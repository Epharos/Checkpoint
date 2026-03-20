#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "ShaderStage.hpp"

namespace cp
{
	class IDescriptorSetLayout;

	struct PushConstantRange
	{
		ShaderStage stages = ShaderStage::None;
		uint32_t offsetBytes = 0;
		uint32_t sizeBytes = 0;
	};

	struct PipelineLayoutInfo
	{
		std::vector<std::shared_ptr<IDescriptorSetLayout>> setLayouts;
		std::vector<PushConstantRange> pushConstants;
	};

	class IPipelineLayout
	{
	public:
		explicit IPipelineLayout(const PipelineLayoutInfo& _info) : info(_info) {}
		virtual ~IPipelineLayout() = default;

	protected:
		const PipelineLayoutInfo& info;
	};
}

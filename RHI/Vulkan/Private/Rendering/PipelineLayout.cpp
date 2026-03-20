#include "../pch.hpp"

#include "PipelineLayout.hpp"

#include "../Core/Device.hpp"
#include "../Data/DescriptorSetLayout.hpp"
#include "../Utilities/VulkanConverter.hpp"

namespace cp
{
	PipelineLayout::PipelineLayout(const PipelineLayoutInfo& _info, Device& _device)
		: IPipelineLayout(_info), device(_device)
	{
		Initialize();
	}

	PipelineLayout::~PipelineLayout()
	{
		Cleanup();
	}

	void PipelineLayout::Initialize()
	{
		std::vector<vk::DescriptorSetLayout> setLayouts;
		setLayouts.reserve(info.setLayouts.size());

		for (const auto& setLayout : info.setLayouts)
		{
			CP_EXPECT_MSG(setLayout, "PipelineLayout contains a null descriptor set layout");
			setLayouts.push_back(static_cast<const DescriptorSetLayout&>(*setLayout).GetHandle());
		}

		std::vector<vk::PushConstantRange> pushConstantRanges;
		pushConstantRanges.reserve(info.pushConstants.size());

		for (const auto& [stages, offsetBytes, sizeBytes] : info.pushConstants)
		{
			vk::PushConstantRange range;
			range.setStageFlags(EnumBitsCast<vk::ShaderStageFlags>(stages));
			range.setOffset(offsetBytes);
			range.setSize(sizeBytes);

			pushConstantRanges.push_back(range);
		}

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.setSetLayoutCount(static_cast<uint32_t>(setLayouts.size()));
		pipelineLayoutCreateInfo.setPSetLayouts(setLayouts.data());
		pipelineLayoutCreateInfo.setPushConstantRangeCount(static_cast<uint32_t>(pushConstantRanges.size()));
		pipelineLayoutCreateInfo.setPPushConstantRanges(pushConstantRanges.data());

		CP_VK_CHECK(device.GetHandle().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
		CP_ENSURE_MSG(pipelineLayout != VK_NULL_HANDLE, "PipelineLayout was not initialized");
	}

	void PipelineLayout::Cleanup() const
	{
		if (pipelineLayout != VK_NULL_HANDLE)
		{
			device.GetHandle().destroyPipelineLayout(pipelineLayout);
		}
	}
}

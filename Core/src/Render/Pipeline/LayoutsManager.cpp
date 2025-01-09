#include "pch.hpp"
#include "LayoutsManager.hpp"

namespace Pipeline
{
	LayoutsManager::LayoutsManager(vk::Device _device) : device(_device)
	{

	}

	vk::PipelineLayout LayoutsManager::GetOrCreateLayout(const std::vector<vk::DescriptorSetLayout>& _descriptorSetLayouts, const std::vector<vk::PushConstantRange>& _pushConstantRanges)
	{
		std::size_t hash = HashLayout(_descriptorSetLayouts, _pushConstantRanges);

		if (layouts.find(hash) == layouts.end())
		{
			LOG_TRACE(MF("Creating new pipeline layout [", hash, "]"));

			vk::PipelineLayoutCreateInfo createInfo = {
				{},
				static_cast<std::uint32_t>(_descriptorSetLayouts.size()),
				_descriptorSetLayouts.data(),
				static_cast<std::uint32_t>(_pushConstantRanges.size()),
				_pushConstantRanges.data()
			};

			layouts[hash] = device.createPipelineLayout(createInfo);
		}

		return layouts[hash];
	}

	void LayoutsManager::Cleanup()
	{
		for (auto& layout : layouts)
		{
			device.destroyPipelineLayout(layout.second);
		}
	}

	std::size_t LayoutsManager::HashLayout(const std::vector<vk::DescriptorSetLayout>& _descriptorSetLayouts, const std::vector<vk::PushConstantRange>& _pushConstantRanges) const
	{
		std::size_t hash = 0;

		for (const auto& layout : _descriptorSetLayouts)
		{
			hash ^= std::hash<std::uint64_t>()(reinterpret_cast<std::uint64_t>(VkDescriptorSetLayout(layout))) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}

		for (const auto& range : _pushConstantRanges)
		{
			hash ^= std::hash<std::uint32_t>()(range.offset) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<std::uint32_t>()(range.size) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<std::uint32_t>()(VkShaderStageFlags(range.stageFlags)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}

		return hash;
	}
}
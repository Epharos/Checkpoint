#pragma once

#include "../../pch.hpp"

namespace cp
{
	class LayoutsManager
	{
	private:
		vk::Device device;

		std::unordered_map<std::size_t, vk::PipelineLayout> layouts;

		std::size_t HashLayout(const std::vector<vk::DescriptorSetLayout>& _descriptorSetLayouts, const std::vector<vk::PushConstantRange>& _pushConstantRanges) const;

	public:
		LayoutsManager(vk::Device _device);

		vk::PipelineLayout GetOrCreateLayout(const std::vector<vk::DescriptorSetLayout>& _descriptorSetLayouts, const std::vector<vk::PushConstantRange>& _pushConstantRanges);

		void Cleanup();
	};
}
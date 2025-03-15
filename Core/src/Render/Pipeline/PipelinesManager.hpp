#pragma once

#include "../../pch.hpp"

namespace cp
{
	struct PipelineData
	{
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

		vk::RenderPass renderPass;

		std::string shaderFile;
		std::vector<std::pair<vk::ShaderStageFlagBits, std::string>> mains;
	};

	struct PipelineConfig
	{
		std::string name;
		// TODO : Populate with more data such as bools for depth testing, alpha blending, etc.

		bool operator==(const PipelineConfig& other) const
		{
			return name == other.name; //TODO : Add more checks when more data is added
		}
	};

	struct PipelineConfigHasher
	{
		std::size_t operator()(const PipelineConfig& config) const
		{
			return std::hash<std::string>()(config.name); //TODO : Add more hashes when more data is added
		}
	};

	struct PipelineCreateData
	{
		PipelineConfig config;
		vk::GraphicsPipelineCreateInfo createInfo;
		std::string shaderFile;
		std::vector<std::pair<vk::ShaderStageFlagBits, std::string>> mains;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	};

	class PipelinesManager
	{
	protected:
		std::unordered_map<PipelineConfig, PipelineData, PipelineConfigHasher> pipelines;

		vk::Device device;

	public:
		PipelinesManager(vk::Device device);

		const PipelineData& CreatePipeline(PipelineCreateData& _pipelineData);

		const PipelineData& GetPipeline(const PipelineCreateData& _pipelineData) const;

		void Cleanup();
	};
}
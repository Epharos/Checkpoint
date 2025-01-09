#include "pch.hpp"
#include "PipelinesManager.hpp"

namespace Pipeline
{
	PipelinesManager::PipelinesManager(vk::Device _device)
		: device(_device)
	{

	}

	const PipelineData& PipelinesManager::CreatePipeline(const PipelineCreateData& _pipelineData)
	{
		PipelineData data;

		LOG_TRACE(MF("Creating new pipeline [", _pipelineData.config.name, "]"));

		data.shaderFile = _pipelineData.shaderFile;
		data.mains = _pipelineData.mains;

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		auto code = Helper::File::ReadShaderFile(_pipelineData.shaderFile);
		vk::ShaderModule shaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.data())));

		for (const auto& path : _pipelineData.mains)
		{
			shaderStages.push_back(vk::PipelineShaderStageCreateInfo({}, path.first, shaderModule, path.second.c_str()));
		}

		data.pipeline = device.createGraphicsPipeline({}, _pipelineData.createInfo, nullptr).value;
		data.pipelineLayout = _pipelineData.createInfo.layout;
		data.descriptorSetLayouts = _pipelineData.descriptorSetLayouts;

		pipelines[_pipelineData.config] = data;

		return pipelines[_pipelineData.config];
	}

	const PipelineData& PipelinesManager::GetPipeline(const PipelineCreateData& _pipelineData) const
	{
		return pipelines.at(_pipelineData.config);
	}

	void PipelinesManager::Cleanup()
	{
		for (auto& pipeline : pipelines)
		{
			device.destroyPipeline(pipeline.second.pipeline);
		}
	}
}
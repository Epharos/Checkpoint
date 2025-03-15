#include "pch.hpp"
#include "PipelinesManager.hpp"

namespace cp
{
	PipelinesManager::PipelinesManager(vk::Device _device)
		: device(_device)
	{

	}

	const PipelineData& PipelinesManager::CreatePipeline(PipelineCreateData& _pipelineData)
	{
		PipelineData data;

		LOG_TRACE(MF("Creating new pipeline [", _pipelineData.config.name, "]"));

		data.shaderFile = _pipelineData.shaderFile;
		data.mains = _pipelineData.mains;

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		auto code = Helper::File::ReadShaderFile(_pipelineData.shaderFile);

		vk::ShaderModule shaderModule;
		vk::Result smResult = device.createShaderModule(new vk::ShaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.data())), nullptr, &shaderModule);

		if (smResult != vk::Result::eSuccess)
		{
			LOG_ERROR(MF("Error creating the shader module code", smResult));
		}

		for (const auto& path : _pipelineData.mains)
		{
			shaderStages.push_back(vk::PipelineShaderStageCreateInfo({}, path.first, shaderModule, path.second.c_str()));
		}

		_pipelineData.createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		_pipelineData.createInfo.pStages = shaderStages.data();

		data.pipeline = device.createGraphicsPipeline({}, _pipelineData.createInfo, nullptr).value;
		data.pipelineLayout = _pipelineData.createInfo.layout;
		data.descriptorSetLayouts = _pipelineData.descriptorSetLayouts;

		device.destroyShaderModule(shaderModule);

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
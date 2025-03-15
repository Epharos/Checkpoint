#include "pch.hpp"

#include "Material.hpp"

cp::Material::Material(const cp::PipelineData& _pipeline, const vk::DescriptorSetLayout& _descriptorSetLayout, const cp::VulkanContext* _context) :
	pipelineData(&_pipeline), descriptorSetLayout(_descriptorSetLayout), context(_context)
{

}

void cp::Material::BindMaterial(vk::CommandBuffer& _command)
{
	_command.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData->pipeline);
}

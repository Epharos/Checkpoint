#include "pch.hpp"

#include "Material.hpp"

Resource::Material::Material(const Pipeline::PipelineData& _pipeline, const vk::DescriptorSetLayout& _descriptorSetLayout, const Context::VulkanContext* _context) :
	pipelineData(&_pipeline), descriptorSetLayout(_descriptorSetLayout), context(_context)
{

}

void Resource::Material::BindMaterial(vk::CommandBuffer& _command)
{
	_command.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData->pipeline);
}

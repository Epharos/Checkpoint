#include "pch.hpp"

#include "Material.hpp"

std::unordered_map<cp::MaterialFieldType, size_t> cp::Material::MaterialFieldSizeMap = {
	{ cp::MaterialFieldType::BOOL, sizeof(bool) },
	{ cp::MaterialFieldType::FLOAT, sizeof(float) },
	{ cp::MaterialFieldType::INT, sizeof(int) },
	{ cp::MaterialFieldType::VEC2, sizeof(glm::vec2) },
	{ cp::MaterialFieldType::VEC3, sizeof(glm::vec3) },
	{ cp::MaterialFieldType::VEC4, sizeof(glm::vec4) },
	{ cp::MaterialFieldType::MAT4, sizeof(glm::mat4) }
};

cp::Material::Material(const cp::PipelineData& _pipeline, const vk::DescriptorSetLayout& _descriptorSetLayout, const cp::VulkanContext* _context) :
	pipelineData(&_pipeline), descriptorSetLayout(_descriptorSetLayout), context(_context)
{

}

void cp::Material::BindMaterial(vk::CommandBuffer& _command)
{
	_command.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData->pipeline);
}

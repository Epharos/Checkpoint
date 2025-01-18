#include "pch.hpp"
#include "ColorMaterial.hpp"

ColorMaterial::ColorMaterial(Resource::Material* _material, const Context::VulkanContext*& _context, glm::vec4 _color) : 
	Resource::MaterialInstance(_material, _context), color(_color)
{
	
}

ColorMaterial::~ColorMaterial()
{
	Helper::Memory::DestroyBuffer(context->GetDevice(), buffer, bufferMemory);
}

void ColorMaterial::PopulateDescriptorSet()
{
	descriptorSet = context->GetDescriptorSetManager()->CreateOrphanedDescriptorSet(material->GetDescriptorSetLayout());

	buffer = Helper::Memory::CreateBuffer(context->GetDevice(), context->GetPhysicalDevice(), sizeof(glm::vec4), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferMemory);

	Pipeline::DescriptorSetUpdate update = {};
	update.buffer = buffer;
	update.offset = 0;
	update.range = sizeof(glm::vec4);
	update.dstBinding = 0;
	update.dstArrayElement = 0;
	update.descriptorType = vk::DescriptorType::eUniformBuffer;
	update.descriptorCount = 1;

	context->GetDescriptorSetManager()->UpdateOrphanedDescriptorSet(descriptorSet, update);
	Helper::Memory::MapMemory(context->GetDevice(), bufferMemory, sizeof(glm::vec4), &color);
}

void ColorMaterial::BindMaterialInstance(vk::CommandBuffer _command)
{
	_command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material->GetPipelineLayout(), 2, descriptorSet, nullptr);
}

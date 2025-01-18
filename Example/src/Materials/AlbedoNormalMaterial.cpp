#include "pch.hpp"
#include "AlbedoNormalMaterial.hpp"

AlbedoNormalMaterial::AlbedoNormalMaterial(Resource::Material* _material, const Context::VulkanContext*& _context, Resource::Texture* _albedoTexture, Resource::Texture* _normalTexture) :
	Resource::MaterialInstance(_material, _context), albedoTexture(_albedoTexture), normalTexture(_normalTexture)
{

}

void AlbedoNormalMaterial::PopulateDescriptorSet()
{
	descriptorSet = context->GetDescriptorSetManager()->CreateOrphanedDescriptorSet(material->GetDescriptorSetLayout());

	Pipeline::DescriptorSetUpdate descriptorSetUpdate;
	descriptorSetUpdate.updateType = Pipeline::DescriptorSetUpdateType::IMAGE;
	descriptorSetUpdate.dstBinding = 0;
	descriptorSetUpdate.dstArrayElement = 0;
	descriptorSetUpdate.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorSetUpdate.descriptorCount = 1;
	descriptorSetUpdate.imageView = albedoTexture->GetImageView();
	descriptorSetUpdate.sampler = albedoTexture->GetSampler();
	descriptorSetUpdate.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	context->GetDescriptorSetManager()->UpdateOrphanedDescriptorSet(descriptorSet, descriptorSetUpdate);

	descriptorSetUpdate.dstBinding = 1;
	descriptorSetUpdate.imageView = normalTexture->GetImageView();
	descriptorSetUpdate.sampler = normalTexture->GetSampler();

	context->GetDescriptorSetManager()->UpdateOrphanedDescriptorSet(descriptorSet, descriptorSetUpdate);
}

void AlbedoNormalMaterial::BindMaterialInstance(vk::CommandBuffer _command)
{
	_command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material->GetPipelineLayout(), 2, descriptorSet, nullptr);
}

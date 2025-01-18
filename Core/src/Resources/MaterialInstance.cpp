#include "pch.hpp"
#include "MaterialInstance.hpp"

#include "Material.hpp"

Resource::MaterialInstance::MaterialInstance(Material* _material, const Context::VulkanContext*& _context)
	: material(_material), context(_context) 
{
	descriptorSet = context->GetDescriptorSetManager()->CreateOrphanedDescriptorSet(material->GetDescriptorSetLayout());
	LOG_DEBUG(MF("CREATING MATERIAL INSTANCE (", GetMaterial(), ")"));
}

Resource::MaterialInstance::~MaterialInstance()
{
	context->GetDescriptorSetManager()->DestroyOrphanedDescriptorSet(descriptorSet);
}

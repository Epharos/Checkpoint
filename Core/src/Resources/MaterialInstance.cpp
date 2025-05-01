#include "pch.hpp"
#include "MaterialInstance.hpp"

#include "Material.hpp"

cp::MaterialInstance::MaterialInstance(Material* _material, const cp::VulkanContext*& _context)
	: material(_material), context(_context) 
{
	//descriptorSet = context->GetDescriptorSetManager()->CreateOrphanedDescriptorSet(material->GetDescriptorSetLayout());
	LOG_DEBUG(MF("CREATING MATERIAL INSTANCE (", GetMaterial(), ")"));
}

cp::MaterialInstance::~MaterialInstance()
{
	context->GetDescriptorSetManager()->DestroyOrphanedDescriptorSet(descriptorSet);
}

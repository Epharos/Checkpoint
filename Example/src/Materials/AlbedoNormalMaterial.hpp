#pragma once

#include "../pch.hpp"

class AlbedoNormalMaterial : public Resource::MaterialInstance
{
protected:
	vk::Buffer buffer;
	vk::DeviceMemory bufferMemory;

	Resource::Texture* albedoTexture;
	Resource::Texture* normalTexture;

public:
	AlbedoNormalMaterial(Resource::Material* _material, const Context::VulkanContext*& _context, Resource::Texture* _albedoTexture, Resource::Texture* _normalTexture);

	void PopulateDescriptorSet() override;
	void BindMaterialInstance(vk::CommandBuffer _command) override;

	inline void SetAlbedoTexture(Resource::Texture* _albedoTexture) { albedoTexture = _albedoTexture; }
	inline void SetNormalTexture(Resource::Texture* _normalTexture) { normalTexture = _normalTexture; }

	inline constexpr Resource::Texture* GetAlbedoTexture() const { return albedoTexture; }
	inline constexpr Resource::Texture* GetNormalTexture() const { return normalTexture; }
};
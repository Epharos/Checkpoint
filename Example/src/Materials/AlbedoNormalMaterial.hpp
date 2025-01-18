#pragma once

#include "../pch.hpp"

class AlbedoNormalMaterial : public Resource::MaterialInstance
{
protected:
	vk::Buffer buffer;
	vk::DeviceMemory bufferMemory;

	Resource::Texture* albedoTexture;
	Resource::Texture* normalTexture;

	float scale = 1.0f;

public:
	AlbedoNormalMaterial(Resource::Material* _material, const Context::VulkanContext*& _context, Resource::Texture* _albedoTexture, Resource::Texture* _normalTexture, float _scale = 1.0f);
	virtual ~AlbedoNormalMaterial();

	virtual void PopulateDescriptorSet() override;
	virtual void BindMaterialInstance(vk::CommandBuffer _command) override;

	inline void SetAlbedoTexture(Resource::Texture* _albedoTexture) { albedoTexture = _albedoTexture; }
	inline void SetNormalTexture(Resource::Texture* _normalTexture) { normalTexture = _normalTexture; }
	inline void SetScale(float _scale) { scale = _scale; }

	inline constexpr Resource::Texture* GetAlbedoTexture() const { return albedoTexture; }
	inline constexpr Resource::Texture* GetNormalTexture() const { return normalTexture; }
	inline constexpr float GetScale() const { return scale; }
};
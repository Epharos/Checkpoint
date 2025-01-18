#pragma once

#include "../pch.hpp"

class ColorMaterial : public Resource::MaterialInstance
{
protected:
	glm::vec4 color;

	vk::Buffer buffer;
	vk::DeviceMemory bufferMemory;

public:
	ColorMaterial(Resource::Material* _material, const Context::VulkanContext*& _context, glm::vec4 _color);
	virtual ~ColorMaterial();

	void PopulateDescriptorSet() override;
	void BindMaterialInstance(vk::CommandBuffer _command) override;

	inline void SetColor(const glm::vec4& _color) { color = _color; }
	inline constexpr glm::vec4 GetColor() const { return color; }
};
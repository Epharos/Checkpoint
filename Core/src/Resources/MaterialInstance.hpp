#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

namespace cp
{
	class Material;

	class MaterialInstance
	{
	protected:
		Material* material;
		vk::DescriptorSet descriptorSet;

		const cp::VulkanContext* context;

	public:
		MaterialInstance(Material* _material, const cp::VulkanContext*& _context);
		virtual ~MaterialInstance();

		virtual void PopulateDescriptorSet() = 0;

		virtual void BindMaterialInstance(vk::CommandBuffer _command) = 0;

		inline constexpr vk::DescriptorSet GetDescriptorSet() const { return descriptorSet; }
		inline constexpr Material* GetMaterial() const { return material; }
	};
}
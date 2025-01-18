#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

namespace Resource
{
	class Material;

	class MaterialInstance
	{
	protected:
		Material* material;
		vk::DescriptorSet descriptorSet;

		const Context::VulkanContext* context;

	public:
		MaterialInstance(Material* _material, const Context::VulkanContext*& _context);
		virtual ~MaterialInstance();

		virtual void PopulateDescriptorSet() = 0;

		virtual void BindMaterialInstance(vk::CommandBuffer _command) = 0;

		inline constexpr vk::DescriptorSet GetDescriptorSet() const { return descriptorSet; }
		inline constexpr Material* GetMaterial() const { return material; }
	};
}
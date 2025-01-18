#pragma once

#include "../pch.hpp"
#include "MaterialInstance.hpp"
#include "../Context/VulkanContext.hpp"

namespace Resource
{
	class Material
	{
	protected:
		const Pipeline::PipelineData* pipelineData;
		const vk::DescriptorSetLayout descriptorSetLayout;

		const Context::VulkanContext* context;

	public:
		Material(const Pipeline::PipelineData& _pipeline, const vk::DescriptorSetLayout& _descriptorSetLayout, const Context::VulkanContext* _context);

		template <class T, typename... Args>
		T* CreateMaterialInstance(Args&& ... args)
		{
#ifdef _DEBUG
			if (!std::is_base_of<MaterialInstance, T>::value)
				throw std::runtime_error("MaterialInstance must be a base of T");
#endif

			T* instance = new T(this, context, std::forward<Args>(args)...);
			instance->PopulateDescriptorSet();
			return instance;
		}

		inline constexpr vk::Pipeline GetPipeline() const { return pipelineData->pipeline; }
		inline constexpr vk::PipelineLayout GetPipelineLayout() const { return pipelineData->pipelineLayout; }
		inline constexpr vk::DescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }

		virtual void BindMaterial(vk::CommandBuffer& _command);
	};
}
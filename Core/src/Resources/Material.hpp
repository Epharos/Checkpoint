#pragma once

#include "../pch.hpp"
#include "MaterialInstance.hpp"
#include "../Context/VulkanContext.hpp"

#include "../Util/Serializers/Serializable.hpp"

namespace cp
{
	enum class MaterialFieldType
	{
		BOOL,
		FLOAT,
		INT,
		VEC2,
		VEC3,
		VEC4,
		MAT4,
		TEXTURE
	};

	struct MaterialField
	{
		MaterialFieldType type;
		std::string name;
		size_t offset;
	};

	struct RenderPassRequirement
	{
		//RenderPassID renderPassID;
		bool requireUniquePipeline;
	};

	class Material : public ISerializable
	{
	protected:
		static std::unordered_map<MaterialFieldType, size_t> MaterialFieldSizeMap;

		std::vector<MaterialField> fields;
		vk::DescriptorSetLayout layout;

		const cp::VulkanContext* context;

	public:
		Material(const cp::PipelineData& _pipeline, const vk::DescriptorSetLayout& _descriptorSetLayout, const cp::VulkanContext* _context);

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

		void Reload(); //TODO : Create 2 pipelines, one for the shadow pass and one for the main pass
		// REFLEXION : Should I use Dynamic Rendering ? Maybe not now, maybe later

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;
	};
}
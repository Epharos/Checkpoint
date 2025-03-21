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

		std::string name = "Material";
		std::string shaderPath = "";

		std::vector<MaterialField> fields;
		vk::DescriptorSetLayout layout;

		const cp::VulkanContext* context;

		std::vector<vk::DescriptorSetLayoutBinding> GenerateBindings();

	public:
		Material(const cp::VulkanContext* _context);

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

		inline constexpr vk::DescriptorSetLayout GetDescriptorSetLayout() const { return layout; }

		virtual void BindMaterial(vk::CommandBuffer& _command);

		virtual void Reload(); // REFLEXION : Should I use Dynamic Rendering ? Not now, maybe later

		virtual void Serialize(ISerializer& _serializer) const override;
		virtual void Deserialize(ISerializer& _serializer) override;

		const std::string& GetName() const { return name; }
		std::string* GetNamePtr() { return &name; }

		const std::string& GetShaderPath() const { return shaderPath; }
		void SetShaderPath(const std::string& _path) { shaderPath = _path; }
	};
}
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

	enum class ShaderStages : uint16_t
	{
		Vertex = 1 << 0,
		Fragment = 1 << 1,
		Geometry = 1 << 2,
		TessellationControl = 1 << 3,
		TessellationEvaluation = 1 << 4,
		Mesh = 1 << 5, //Experimental

		Compute = 1 << 6,

		RayGen = 1 << 7,
		AnyHit = 1 << 8,
		ClosestHit = 1 << 9,
		Miss = 1 << 10,
		Intersection = 1 << 11,

		Pixel = Fragment, // Alias
		AllGraphics = Vertex | Fragment | Geometry | TessellationControl | TessellationEvaluation | Mesh,
		RayTracing = RayGen | AnyHit | ClosestHit | Miss | Intersection
	};

	struct RenderPassRequirement : public ISerializable
	{
		bool renderToPass;
		std::string customShaderPath;
		std::unordered_map<ShaderStages, std::string> customEntryPoints;

		void Serialize(ISerializer& _serializer) const override;

		void Deserialize(ISerializer& _serializer) override;
	};

	class Material : public ISerializable
	{
	protected:
		static std::unordered_map<MaterialFieldType, size_t> MaterialFieldSizeMap;

		std::string name = "Material";
		std::string shaderPath = "";

		uint16_t shaderStages = 0;

		std::vector<MaterialField> fields;
		vk::DescriptorSetLayout layout;

		std::unordered_map<std::string, RenderPassRequirement> rpRequirements;

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

		inline bool HasShaderStage(const ShaderStages& _stage) const { return (shaderStages & static_cast<uint16_t>(_stage)) != 0; }
		inline void AddShaderStage(const ShaderStages& _stage) { shaderStages |= static_cast<uint16_t>(_stage); }
		inline void RemoveShaderStage(const ShaderStages& _stage) { shaderStages &= ~static_cast<uint16_t>(_stage); }

		virtual void Reload(); // REFLEXION : Should I use Dynamic Rendering ? Not now, maybe later

		virtual void Serialize(ISerializer& _serializer) const override;
		virtual void Deserialize(ISerializer& _serializer) override;

		const std::string& GetName() const { return name; }
		std::string* GetNamePtr() { return &name; }

		const std::string& GetShaderPath() const { return shaderPath; }
		void SetShaderPath(const std::string& _path) { shaderPath = _path; }

		inline RenderPassRequirement& GetRenderPassRequirement(const std::string& _rpName) { return rpRequirements[_rpName]; }
	};
}
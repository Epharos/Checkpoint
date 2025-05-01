#pragma once

#include "../pch.hpp"
#include "MaterialInstance.hpp"
#include "../Context/VulkanContext.hpp"

#include "../Util/Serializers/Serializable.hpp"

namespace cp
{
	class Renderer;

	template<typename T>
	concept MaterialInstanceType = std::is_base_of_v<T, MaterialInstance>;

	enum class MaterialFieldType : uint8_t
	{
		BOOL,
		HALF,
		FLOAT,
		INT,
		UINT,
		VECTOR,
		MATRIX,
	};

	enum class BindingType : uint8_t
	{
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		TEXTURE,
		//SAMPLER,
		//IMAGE_SAMPLER,
	};

	struct PipelineCreationData
	{
		//Vertex Input
		// VI Format
		// Input Rate
		// Binding Slots

		//Rasterization
		vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
		vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
		vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;
		bool depthBiasEnable = false;
		float depthBiasConstantFactor = 0.0f;
		float depthBiasClamp = 0.0f;
		float depthBiasSlopeFactor = 0.0f;

		//Multisampling
		vk::SampleCountFlagBits rasterizationSamples = vk::SampleCountFlagBits::e1;
		bool sampleShadingEnable = false;

		//Depth Stencil
		bool depthTestEnable = true;
		bool depthWriteEnable = true;
		vk::CompareOp depthCompareOp = vk::CompareOp::eLess;
		bool stencilTestEnable = false;
		vk::StencilOpState operations;

		//Blending
		bool enableBlending = false;
		vk::BlendFactor srcColorBlendFactor = vk::BlendFactor::eOne;
		vk::BlendFactor dstColorBlendFactor = vk::BlendFactor::eZero;
		vk::BlendOp colorBlendOp = vk::BlendOp::eAdd;
		vk::BlendFactor srcAlphaBlendFactor = vk::BlendFactor::eOne;
		vk::BlendFactor dstAlphaBlendFactor = vk::BlendFactor::eZero;
		vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd;
	};

	struct MaterialField : public ISerializable
	{
		MaterialFieldType type;
		std::string name; // Will only be used in the editor
		size_t offset;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		std::string* GetNamePtr() { return &name; }
	};

	struct MaterialBinding : public ISerializable
	{
		BindingType type;
		std::string name; // Will only be used in the editor
		uint8_t index;
		uint16_t shaderStages;
		std::vector<MaterialField> fields;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		bool HasShaderStage(const ShaderStages& _stage) const { return (shaderStages & static_cast<uint16_t>(_stage)) != 0; }
		void AddShaderStage(const ShaderStages& _stage) { shaderStages |= static_cast<uint16_t>(_stage); }
		void RemoveShaderStage(const ShaderStages& _stage) { shaderStages &= ~static_cast<uint16_t>(_stage); }

		void AddField(const MaterialField& _field) { fields.push_back(_field); }

		std::string* GetNamePtr() { return &name; }

		bool RemoveField(const MaterialField& _field);
	};

	struct MaterialDescriptor : public ISerializable
	{
		uint8_t index;
		std::string name; // Will only be used in the editor

		vk::DescriptorSetLayout layout;

		std::vector<MaterialBinding> bindings;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		void GenerateDescriptorSetLayout(const cp::VulkanContext* _context);
		std::string* GetNamePtr() { return &name; }

		bool RemoveBinding(const MaterialBinding& _binding);
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
		bool useDefaultShader;
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

		std::vector<MaterialDescriptor> descriptors;
		PipelineCreationData pipelineCreationData;
		std::unordered_map<std::string, RenderPassRequirement> rpRequirements;

		std::unordered_map<std::string, cp::PipelineData*> pipelineDatas; // The pipeline data used by the material (if we need to reload the material)

		const cp::VulkanContext* context;

	public:
		Material(const cp::VulkanContext* _context);

		template <MaterialInstanceType T, typename... Args>
		T* CreateMaterialInstance(Args&& ... args)
		{
			T* instance = new T(this, context, std::forward<Args>(args)...);
			instance->PopulateDescriptorSet();
			return instance;
		}

		virtual void BindMaterial(vk::CommandBuffer& _command);

		inline bool HasShaderStage(const ShaderStages& _stage) const { return (shaderStages & static_cast<uint16_t>(_stage)) != 0; }
		inline void AddShaderStage(const ShaderStages& _stage) { shaderStages |= static_cast<uint16_t>(_stage); }
		inline void RemoveShaderStage(const ShaderStages& _stage) { shaderStages &= ~static_cast<uint16_t>(_stage); }

		virtual void Reload(cp::Renderer& _renderer);

		virtual void Serialize(ISerializer& _serializer) const override;
		virtual void Deserialize(ISerializer& _serializer) override;

		const std::string& GetName() const { return name; }
		std::string* GetNamePtr() { return &name; }

		const std::string& GetShaderPath() const { return shaderPath; }
		void SetShaderPath(const std::string& _path) { shaderPath = _path; }

		inline RenderPassRequirement& GetRenderPassRequirement(const std::string& _rpName) { return rpRequirements[_rpName]; }

		inline std::vector<MaterialDescriptor>& GetDescriptors() { return descriptors; }
		inline void AddDescriptor(const MaterialDescriptor& _descriptor) { descriptors.push_back(_descriptor); }

		inline uint16_t GetShaderStages() const { return shaderStages; }
	};
}
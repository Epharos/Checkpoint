#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

#include "../Util/Serializers/Serializable.hpp"

#include "../Util/ShaderCompiler/SlangCompiler.hpp"

namespace cp
{
	class Material;

	struct MaterialInstanceField : public ISerializable
	{
		std::string name;
		std::vector<uint8_t> data;
		const cp::ShaderField* associatedField; // Pointer to the associated field in the material

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		void* GetDataPtr() const { return (void*)data.data(); }
	};

	struct MaterialInstanceResource : public ISerializable
	{
		const cp::VulkanContext* context;

		std::string name;
		cp::ShaderResourceKind kind = cp::ShaderResourceKind::Unknown;
		uint32_t binding = 0;
		uint32_t set = 0;

		const cp::ShaderResource* associatedResource; // Pointer to the associated resource in the material

		std::vector<MaterialInstanceField> fields; // Fields that are part of this resource
		std::vector<uint8_t> packedData;

		cp::Buffer dataBuffer;

		MaterialInstanceResource(const cp::VulkanContext* _context, const cp::ShaderResource* _sr) : context(_context), associatedResource(_sr) {};
		MaterialInstanceResource(const cp::VulkanContext* _context) : context(_context), associatedResource(nullptr) {};
		MaterialInstanceResource(const cp::MaterialInstanceResource& other);
		~MaterialInstanceResource();

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		void CollectFields(const ShaderField& field, const std::string& prefix, std::vector<MaterialInstanceField>& fields) const;

		void Repack();

		void CreateBuffer();
		void UpdateBufferData();

		MaterialInstanceResource& operator=(const MaterialInstanceResource& other);
	};

	class MaterialInstance : public ISerializable
	{
	protected:
		std::string associatedMaterial;
		std::shared_ptr<Material> material;

		std::vector<MaterialInstanceResource> resources; // Resources that are part of this material instance

		const VulkanContext* context;

	public:
		MaterialInstance(const VulkanContext* _context);
		virtual ~MaterialInstance();

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		[[nodiscard]] std::vector<MaterialInstanceResource> CreateMaterialInstanceResources();

		void ValidateData();

#ifdef IN_EDITOR
		QWidget* CreateMaterialInstanceWidget(QWidget* _parent);
#endif

		void BindMaterialInstance(vk::CommandBuffer _command, const std::string& _renderpass);
		void UpdateDescriptorSets();

		inline std::shared_ptr<Material> GetMaterial() const { return material; }
		inline std::string GetAssociatedMaterial() const { return associatedMaterial; }
		inline std::string GetDescriptorSetName(const uint32_t& _set) const;

		inline void SetAssociatedMaterial(const std::string& _materialPath) { associatedMaterial = _materialPath; }

		static std::shared_ptr<MaterialInstance> LoadMaterialInstance(const VulkanContext& _context, const std::string& _path);
	};
}
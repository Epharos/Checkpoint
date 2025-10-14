#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

#include "../Util/Serializers/Serializable.hpp"

#include "../Util/ShaderCompiler/SlangCompiler.hpp"

namespace cp
{
	class Material;
	/*struct MaterialDescriptor;
	struct MaterialBinding;
	struct MaterialField;
	class Texture;

	struct MaterialInstanceField : public ISerializable
	{
		std::string name;
		std::vector<uint8_t> data;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		MaterialField& GetAssociatedField(const MaterialBinding& _material);

#ifdef IN_EDITOR
		QWidget* CreateFieldWidget(QWidget* _parent, const MaterialBinding& _material);
#endif
	};

	struct MaterialInstanceBinding : public ISerializable
	{
		std::string name;
		std::unordered_map<std::string, MaterialInstanceField> fields;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		MaterialBinding& GetAssociatedBinding(const MaterialDescriptor& _material);
		void SynchronizeFieldsWithMaterial(const MaterialDescriptor& _material);

#ifdef IN_EDITOR
		QWidget* CreateBindingWidget(QWidget* _parent, const MaterialDescriptor& _material);
#endif
	};

	struct MaterialInstanceDescriptor : public ISerializable
	{
		std::string name;
		std::unordered_map<std::string, MaterialInstanceBinding> bindings;

		vk::DescriptorSet descriptorSet;

		void Serialize(ISerializer& _serializer) const;
		void Deserialize(ISerializer& _serializer) override;

		MaterialDescriptor& GetAssociatedDescriptor(const Material& _material);
		void SynchronizeBindingsWithMaterial(const Material& _material);

#ifdef IN_EDITOR
		QWidget* CreateDescriptorWidget(QWidget* _parent, const Material& _material);
#endif
	};*/

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
		std::string name;
		cp::ShaderResourceKind kind = cp::ShaderResourceKind::Unknown;
		uint32_t binding = 0;
		uint32_t set = 0;

		const cp::ShaderResource* associatedResource; // Pointer to the associated resource in the material

		std::vector<MaterialInstanceField> fields; // Fields that are part of this resource
		std::vector<uint8_t> packedData;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		void CollectFields(const ShaderField& field, const std::string& prefix, std::vector<MaterialInstanceField>& fields) const;

		void Repack();
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

		/*virtual void PopulateDescriptorSet() = 0;

		virtual void BindMaterialInstance(vk::CommandBuffer _command) = 0;*/

		inline std::shared_ptr<Material> GetMaterial() const { return material; }
		inline std::string GetAssociatedMaterial() const { return associatedMaterial; }

		inline void SetAssociatedMaterial(const std::string& _materialPath) { associatedMaterial = _materialPath; }
	};
}
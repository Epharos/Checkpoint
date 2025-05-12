#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

#include "../Util/Serializers/Serializable.hpp"

namespace cp
{
	class Material;
	struct MaterialDescriptor;
	struct MaterialBinding;
	struct MaterialField;
	class Texture;

	struct MaterialInstanceField : public ISerializable
	{
		std::string name;
		std::vector<uint8_t> data;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		MaterialField& GetAssociatedField(MaterialBinding& _material);
	};

	struct MaterialInstanceBinding : public ISerializable
	{
		std::string name;
		std::unordered_map<std::string, MaterialInstanceField> fields;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		MaterialBinding& GetAssociatedBinding(const MaterialDescriptor& _material);
		void SynchronizeFieldsWithMaterial(const MaterialDescriptor& _material);
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
	};

	class MaterialInstance : public ISerializable
	{
	protected:
		std::string associatedMaterial;
		std::shared_ptr<Material> material;

		std::unordered_map<std::string, MaterialInstanceDescriptor> descriptorSets;

		const cp::VulkanContext* context;

	public:
		MaterialInstance(const cp::VulkanContext*& _context);
		virtual ~MaterialInstance();

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		void SynchronizeDescriptorSetsWithMaterial();

		virtual void PopulateDescriptorSet() = 0;

		virtual void BindMaterialInstance(vk::CommandBuffer _command) = 0;

		inline std::shared_ptr<Material> GetMaterial() const { return material; }
	};
}
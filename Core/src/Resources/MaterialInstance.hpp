#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

#include "../Util/Serializers/Serializable.hpp"

#include "../Util/ShaderCompiler/SlangCompiler.hpp"

#include "Texture.hpp"

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
		const VulkanContext* context;

		std::string name;
		cp::ShaderResourceKind kind = cp::ShaderResourceKind::Unknown;
		uint32_t binding = 0;
		uint32_t set = 0;

		const cp::ShaderResource* associatedResource = nullptr; // Pointer to the associated resource in the material

		std::vector<MaterialInstanceField> fields; // Fields that are part of this resource
		std::vector<uint8_t> packedData;

		// For constant buffers and storage buffers, we need to pack the data into a single buffer
		cp::Buffer packedBuffer;

		// For textures and samplers, we need to have access to the actual resource
		std::string associatedTexture; // Path to the associated texture

		MaterialInstanceResource(const VulkanContext* _context) : context(_context) {}
		~MaterialInstanceResource();

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		void CollectFields(const ShaderField& field, const std::string& prefix, std::vector<MaterialInstanceField>& fields) const;

		void Repack();

		void InitBuffer();
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

		inline std::shared_ptr<Material> GetMaterial() const { return material; }
		inline std::string GetAssociatedMaterial() const { return associatedMaterial; }

		inline void SetAssociatedMaterial(const std::string& _materialPath) { associatedMaterial = _materialPath; }
	};
}
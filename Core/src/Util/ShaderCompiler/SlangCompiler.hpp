#pragma once

#include "../../pch.hpp"
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>

#include "../Serializers/Serializable.hpp"
#include "../Serializers/ISerializer.hpp"

using namespace Slang;
using namespace slang;

namespace cp
{
	class Material;

	enum class ShaderResourceKind : uint8_t
	{
		ConstantBuffer,
		StructuredBuffer,
		Sampler,
		TextureResource,
		CombinedImageSampler,
		PushConstant,
		Unknown
	};

	struct ShaderField : public ISerializable
	{
		std::string name;
		std::string typeName;
		size_t size;
		size_t offset = -1;
		size_t alignment;
		size_t stride;
		std::vector<ShaderField> fields;

		std::string vectorType; // For vector types, e.g. float, int, ...
		std::string matrixType; // For matrix types, e.g. float, int, ...
		size_t vectorSize = 0; // Number of components in the vector
		size_t matrixRows = 0; // Number of rows in the matrix
		size_t matrixColumns = 0; // Number of columns in the matrix

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;
	};

	struct ShaderResource : public ISerializable
	{
		std::string name;
		ShaderResourceKind kind = ShaderResourceKind::Unknown;
		uint32_t binding;
		uint32_t set;
		std::string typeName;
		ShaderField field;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;
	};

	struct EntryPoint : public ISerializable
	{
		std::string name;
		ShaderStages stage;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;
	};

	struct ShaderReflection : public ISerializable
	{
		std::vector<ShaderResource> resources;
		std::vector<EntryPoint> entryPoints;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;
		//TODO : Make destructor
	};

	class SlangCompiler
	{
	protected:
		ComPtr<IGlobalSession> globalSession;

		ShaderResourceKind DetermineResourceKind(TypeLayoutReflection* layout);
		ShaderField ExtractFieldInfo(TypeLayoutReflection* typeLayout);

	public:
		SlangCompiler();
		bool CompileMaterialSlangToSpirV(Material& _material);

#ifdef IN_EDITOR
		static QWidget* CreateFieldWidget(const ShaderField& field, QWidget* parent);
		static QWidget* CreateResourceWidget(const ShaderResource& resource, QWidget* parent, const bool& _showEngineSets = false);
#endif
	};
}
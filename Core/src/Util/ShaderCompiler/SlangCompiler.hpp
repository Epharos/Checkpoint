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

	enum ShaderResourceKind
	{
		ConstantBuffer,
		StructuredBuffer,
		Sampler,
		Texture,
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

	struct ShaderReflection : public ISerializable
	{
		std::vector<ShaderResource> resources;

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
		bool CompileMaterialSlangToSpirV(cp::Material& _material);

#ifdef IN_EDITOR
		QWidget* CreateFieldWidget(const ShaderField& field, QWidget* parent);
		QWidget* CreateResourceWidget(const ShaderResource& resource, QWidget* parent);
#endif
	};
}
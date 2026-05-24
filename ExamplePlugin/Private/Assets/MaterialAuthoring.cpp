#include "MaterialAuthoring.hpp"
#include "MaterialSaver.hpp"

#include <Common/IO/FileHelper.hpp>
#include <Resources/AssetRegistry.hpp>
#include <ShaderCompiler/ShaderCompiler.hpp>
#include "../ExamplePluginDir.hpp"

namespace cp
{
	static constexpr std::string_view SectionIdMat = "material";
	static constexpr std::string_view FieldIdShaderPath = "shaderPath";

	static std::string_view BindingKindName(BindingKind _kind)
	{
		switch (_kind)
		{
		case BindingKind::ConstantBuffer: return "Constant Buffer";
		case BindingKind::PushConstant: return "Push Constant";
		case BindingKind::StructuredBuffer: return "Structured Buffer";
		case BindingKind::RWStructuredBuffer: return "RW Structured Buffer";
		case BindingKind::ByteAddressBuffer: return "Byte Address Buffer";
		case BindingKind::RWByteAddressBuffer: return "RW Byte Address Buffer";
		case BindingKind::Texture1D: return "Texture1D";
		case BindingKind::Texture1DArray: return "Texture1D[]";
		case BindingKind::Texture2D: return "Texture2D";
		case BindingKind::Texture2DArray: return "Texture2D[]";
		case BindingKind::Texture2DMS: return "Texture2DMS";
		case BindingKind::Texture2DMSArray: return "Texture2DMS[]";
		case BindingKind::Texture3D: return "Texture3D";
		case BindingKind::TextureCube: return "TextureCube";
		case BindingKind::TextureCubeArray: return "TextureCube[]";
		case BindingKind::RWTexture2D: return "RW Texture2D";
		case BindingKind::Sampler: return "Sampler";
		case BindingKind::SamplerComparison: return "Sampler (comparison)";
		case BindingKind::ParameterBlock: return "Parameter Block";
		case BindingKind::AccelerationStructure: return "Acceleration Structure";
		default: return "Unknown";
		}
	}

	static void AppendAnnotations(std::string& _s, const std::vector<std::string>& _annotations)
	{
		for (const std::string& a : _annotations)
		{
			_s += "  [";
			_s += a;
			_s += "]";
		}
	}

	static std::string FormatFieldValue(const FieldReflection& _field)
	{
		std::string s = _field.typeName;
		s += "  offset=" + std::to_string(_field.offset);
		s += "  size=" + std::to_string(_field.size);

		if (_field.arrayCount > 0)
		{
			s += "  count=" + std::to_string(_field.arrayCount);
			s += "  stride=" + std::to_string(_field.stride);
		}

		AppendAnnotations(s, _field.annotations);
		return s;
	}

	static void AppendFieldRows(
		std::vector<AuthoringFieldDescriptor>& _out,
		const std::vector<FieldReflection>& _fields,
		const std::string& _prefix
	)
	{
		for (const FieldReflection& f : _fields)
		{
			const std::string id = _prefix + f.name;
			_out.push_back({
				.id = id,
				.label = id,
				.valueType = AuthoringValueType::String,
				.inputType = AuthoringInputType::Default,
				.value = FormatFieldValue(f),
				.readOnly = true
			});

			if (f.kind == FieldKind::Struct && !f.fields.empty())
			{
				AppendFieldRows(_out, f.fields, id + ".");
			}
		}
	}

	MaterialAuthoring::MaterialAuthoring(ShaderCompiler& _compiler, AssetRegistry& _assetRegistry)
		: compiler(_compiler)
		, assetRegistry(_assetRegistry)
	{
	}

	std::string_view MaterialAuthoring::FileExtension() const
	{
		return "material";
	}

	const ShaderReflection* MaterialAuthoring::GetOrReflect(const std::filesystem::path& _shaderPath) const
	{
		const std::string key = _shaderPath.string();
		const auto it = reflectionCache.find(key);
		if (it != reflectionCache.end())
		{
			return &it->second;
		}

		const CompileResult result = compiler.ReflectFile(_shaderPath);
		if (!result.success)
		{
			return nullptr;
		}

		auto [ins, ok] = reflectionCache.emplace(key, result.reflection);
		return &ins->second;
	}

	std::vector<AuthoringSectionDescriptor> MaterialAuthoring::BuildSections(
		const std::filesystem::path& _assetPath
	) const
	{
		const auto mat = assetRegistry.Get<Material>().Load(_assetPath);
		if (!mat)
		{
			return {};
		}

		AuthoringSectionDescriptor materialSection;
		materialSection.id = std::string(SectionIdMat);
		materialSection.title = "Material";
		materialSection.fields.push_back({
			.id = std::string(FieldIdShaderPath),
			.label = "Shader",
			.valueType = AuthoringValueType::String,
			.inputType = AuthoringInputType::FilePath,
			.value = mat->shaderPath.string(),
			.readOnly = false,
			.fileExtensions = {"slang"}
		});

		std::vector<AuthoringSectionDescriptor> result;
		result.push_back(std::move(materialSection));

		if (mat->shaderPath.empty())
		{
			return result;
		}

		const std::filesystem::path resolvedShader = mat->shaderPath.is_absolute()
			? mat->shaderPath
			: FindFileFromDirectory(mat->shaderPath.string(), GetExamplePluginDir());

		if (resolvedShader.empty())
		{
			return result;
		}

		const ShaderReflection* reflection = GetOrReflect(resolvedShader);
		if (!reflection)
		{
			return result;
		}

		for (const BindingReflection& binding : reflection->bindings)
		{
			AuthoringSectionDescriptor bindingSection;
			bindingSection.id = "binding_" + binding.name;
			bindingSection.title = binding.name + " — " + std::string(BindingKindName(binding.kind));

			std::string meta = "set=" + std::to_string(binding.set);
			meta += "  binding=" + std::to_string(binding.binding);

			if (binding.size > 0)
			{
				meta += "  size=" + std::to_string(binding.size) + "B";
			}

			if (binding.stride > 0)
			{
				meta += "  stride=" + std::to_string(binding.stride) + "B";
			}

			AppendAnnotations(meta, binding.annotations);

			bindingSection.fields.push_back({
				.id = "_meta",
				.label = "Info",
				.valueType = AuthoringValueType::String,
				.inputType = AuthoringInputType::Default,
				.value = meta,
				.readOnly = true
			});

			AppendFieldRows(bindingSection.fields, binding.fields, "");

			result.push_back(std::move(bindingSection));
		}

		return result;
	}

	bool MaterialAuthoring::ApplyValue(
		const std::filesystem::path& _assetPath,
		std::string_view _sectionId,
		std::string_view _fieldId,
		const AuthoringValue& _value,
		std::string& _outError
	) const
	{
		if (_sectionId != SectionIdMat || _fieldId != FieldIdShaderPath)
		{
			_outError = "Field is read-only.";
			return false;
		}

		const std::string* pathStr = std::get_if<std::string>(&_value);
		if (!pathStr)
		{
			_outError = "Expected a file path string.";
			return false;
		}

		auto mat = assetRegistry.Get<Material>().Load(_assetPath);
		if (!mat)
		{
			_outError = "Failed to load Material: " + _assetPath.string();
			return false;
		}

		reflectionCache.erase(mat->shaderPath.string());

		mat->InvalidateGpuResources();

		mat->shaderPath = *pathStr;
		return MaterialSaver::Save(*mat, _assetPath);
	}
}

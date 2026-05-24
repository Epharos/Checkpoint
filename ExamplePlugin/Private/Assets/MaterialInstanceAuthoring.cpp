#include "MaterialInstanceAuthoring.hpp"
#include "MaterialInstanceSaver.hpp"

#include <Common/IO/FileHelper.hpp>
#include <Resources/AssetRegistry.hpp>
#include <ShaderCompiler/ShaderCompiler.hpp>
#include "../ExamplePluginDir.hpp"
#include "../ExamplePluginAssetRegistry.hpp"

#include <algorithm>

namespace cp
{
	static constexpr std::string_view SectionIdMaterial = "material";
	static constexpr std::string_view SectionIdParameters = "parameters";
	static constexpr std::string_view FieldIdMaterialPath = "materialPath";
	static constexpr std::string_view AnnotationMaterialParam = "MaterialParam";
	static constexpr std::string_view AnnotationColor = "Color";

	static bool HasAnnotation(const std::vector<std::string>& annotations, std::string_view name)
	{
		return std::ranges::any_of(annotations, [name](const std::string& a) { return a == name; });
	}

	static AuthoringValue ParameterValueToAuthoring(const ParameterValue& value)
	{
		return std::visit([](const auto& v) -> AuthoringValue
		{
			using T = std::decay_t<decltype(v)>;

			if constexpr (std::is_same_v<T, bool>)
			{
				return v;
			}
			else if constexpr (std::is_same_v<T, int32_t>)
			{
				return static_cast<int64_t>(v);
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				return static_cast<double>(v);
			}
			else if constexpr (std::is_same_v<T, std::array<float, 2>>)
			{
				return AuthoringVec2{ .x = v[0], .y = v[1] };
			}
			else if constexpr (std::is_same_v<T, std::array<float, 3>>)
			{
				return AuthoringVec3{ .x = v[0], .y = v[1], .z = v[2] };
			}
			else if constexpr (std::is_same_v<T, std::array<float, 4>>)
			{
				return AuthoringVec4{ .x = v[0], .y = v[1], .z = v[2], .w = v[3] };
			}
			else if constexpr (std::is_same_v<T, AssetHandle<ITexture>>)
			{
				return v.IsValid() ? GetRelativePath(v.GetAssetID()).string() : std::string{};
			}

			return std::string{};
		}, value);
	}

	static AuthoringFieldDescriptor MakeFieldFromReflection(
		const FieldReflection& field,
		const ParameterValue* currentValue
	)
	{
		AuthoringFieldDescriptor desc;
		desc.id = field.name;
		desc.label = field.name;
		desc.readOnly = false;

		const bool isColor = HasAnnotation(field.annotations, AnnotationColor);

		if (field.kind == FieldKind::Scalar)
		{
			switch (field.scalar)
			{
			case ScalarKind::Bool:
				desc.valueType = AuthoringValueType::Bool;
				desc.value = currentValue ? ParameterValueToAuthoring(*currentValue) : false;
				break;
			case ScalarKind::Int8:
			case ScalarKind::Int16:
			case ScalarKind::Int32:
			case ScalarKind::Int64:
			case ScalarKind::UInt8:
			case ScalarKind::UInt16:
			case ScalarKind::UInt32:
			case ScalarKind::UInt64:
				desc.valueType = AuthoringValueType::Int;
				desc.value = currentValue ? ParameterValueToAuthoring(*currentValue) : int64_t{0};
				break;
			default:
				desc.valueType = AuthoringValueType::Float;
				desc.value = currentValue ? ParameterValueToAuthoring(*currentValue) : 0.0;
				break;
			}
		}
		else if (field.kind == FieldKind::Vector)
		{
			switch (field.columns)
			{
			case 2:
				desc.valueType = AuthoringValueType::Vec2;
				desc.value = currentValue
					? ParameterValueToAuthoring(*currentValue)
					: AuthoringVec2{};
				break;
			case 3:
				desc.valueType = AuthoringValueType::Vec3;
				desc.inputType = isColor ? AuthoringInputType::Color : AuthoringInputType::Default;
				desc.value = currentValue
					? ParameterValueToAuthoring(*currentValue)
					: AuthoringVec3{};
				break;
			case 4:
				desc.valueType = AuthoringValueType::Vec4;
				desc.inputType = isColor ? AuthoringInputType::Color : AuthoringInputType::Default;
				desc.value = currentValue
					? ParameterValueToAuthoring(*currentValue)
					: AuthoringVec4{};
				break;
			default:
				desc.valueType = AuthoringValueType::Float;
				desc.value = 0.0;
				break;
			}
		}
		else
		{
			desc.valueType = AuthoringValueType::Float;
			desc.value = 0.0;
		}

		return desc;
	}

	MaterialInstanceAuthoring::MaterialInstanceAuthoring(ShaderCompiler& _compiler, AssetRegistry& _assetRegistry)
		: compiler(_compiler)
		, assetRegistry(_assetRegistry)
	{
	}

	std::string_view MaterialInstanceAuthoring::FileExtension() const
	{
		return "matinst";
	}

	const ShaderReflection* MaterialInstanceAuthoring::GetOrCompileReflection(
		const std::filesystem::path& _shaderPath
	) const
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

	std::vector<AuthoringSectionDescriptor> MaterialInstanceAuthoring::BuildSections(
		const std::filesystem::path& _assetPath
	) const
	{
		const auto inst = GetPluginAssetRegistry().Get<MaterialInstance>().Load(_assetPath);
		if (!inst)
		{
			return {};
		}

		AuthoringSectionDescriptor materialSection;
		materialSection.id = std::string(SectionIdMaterial);
		materialSection.title = "Material";
		materialSection.fields.push_back({
			.id = std::string(FieldIdMaterialPath),
			.label = "Shader",
			.valueType = AuthoringValueType::String,
			.inputType = AuthoringInputType::FilePath,
			.value = inst->material.IsValid() ? GetRelativePath(inst->material.GetAssetID()).string() : std::string{},
			.readOnly = false,
			.fileExtensions = { "material" }
		});

		AuthoringSectionDescriptor paramsSection;
		paramsSection.id = std::string(SectionIdParameters);
		paramsSection.title = "Parameters";

		if (inst->material.IsValid())
		{
			const std::filesystem::path& rawShader = inst->material->shaderPath;
			const std::filesystem::path resolvedShader = rawShader.is_absolute()
				? rawShader
				: FindFileFromDirectory(rawShader.string(), GetExamplePluginDir());

			// TODO : Reduce nesting by splitting this block into separate methods

			if (!resolvedShader.empty())
			{
				const ShaderReflection* reflection = GetOrCompileReflection(resolvedShader);

				if (reflection)
				{
					for (const BindingReflection& binding : reflection->bindings)
					{
						if (binding.kind == BindingKind::ConstantBuffer)
						{
							for (const FieldReflection& field : binding.fields)
							{
								if (!HasAnnotation(field.annotations, AnnotationMaterialParam))
								{
									continue;
								}

								const auto paramIt = inst->parameters.find(field.name);
								const ParameterValue* current = paramIt != inst->parameters.end()
									? &paramIt->second
									: nullptr;

								paramsSection.fields.push_back(
									MakeFieldFromReflection(field, current));
							}
						}

						const bool isTexture =
							binding.kind == BindingKind::Texture2D ||
							binding.kind == BindingKind::Texture2DArray ||
							binding.kind == BindingKind::TextureCube;

						if (isTexture && HasAnnotation(binding.annotations, AnnotationMaterialParam))
						{
							const auto paramIt = inst->parameters.find(binding.name);
							std::string texturePath;
							if (paramIt != inst->parameters.end())
							{
								if (const AssetHandle<ITexture>* h = std::get_if<AssetHandle<ITexture>>(&paramIt->second))
								{
									if (h->IsValid())
									{
										texturePath = GetRelativePath(h->GetAssetID()).string();
									}
								}
							}

							paramsSection.fields.push_back({
								.id = binding.name,
								.label = binding.name,
								.valueType = AuthoringValueType::String,
								.inputType = AuthoringInputType::FilePath,
								.value = texturePath,
								.readOnly = false,
								.fileExtensions = { "png", "jpg", "jpeg" }
							});
						}
					}
				}
			}
		}

		return { std::move(materialSection), std::move(paramsSection) };
	}

	bool MaterialInstanceAuthoring::ApplyValue(
		const std::filesystem::path& _assetPath,
		std::string_view _sectionId,
		std::string_view _fieldId,
		const AuthoringValue& _value,
		std::string& _outError
	) const
	{
		auto inst = GetPluginAssetRegistry().Get<MaterialInstance>().Load(_assetPath);
		if (!inst)
		{
			_outError = "Failed to load MaterialInstance: " + _assetPath.string();
			return false;
		}

		if (_sectionId == SectionIdMaterial && _fieldId == FieldIdMaterialPath)
		{
			const std::string* pathStr = std::get_if<std::string>(&_value);
			if (!pathStr)
			{
				_outError = "Expected a file path string.";
				return false;
			}

			if (pathStr->empty())
			{
				inst->material = AssetHandle<Material>{};
			}
			else
			{
				const std::filesystem::path rawPath(*pathStr);
				const std::filesystem::path resolved = rawPath.is_absolute()
					? rawPath
					: ResolveAssetPath(*pathStr);
				inst->material = GetPluginAssetRegistry().Get<Material>().Load(resolved);
				if (!inst->material.IsValid())
				{
					_outError = "Failed to load Material: " + *pathStr;
					return false;
				}
			}

			inst->InvalidateGpuBuffers();

			return MaterialInstanceSaver::Save(*inst, _assetPath);
		}

		if (_sectionId == SectionIdParameters)
		{
			const std::string name(_fieldId);

			if (const bool* v = std::get_if<bool>(&_value))
			{
				inst->parameters[name] = *v;
			}
			else if (const int64_t* v = std::get_if<int64_t>(&_value))
			{
				inst->parameters[name] = static_cast<int32_t>(*v);
			}
			else if (const double* v = std::get_if<double>(&_value))
			{
				inst->parameters[name] = static_cast<float>(*v);
			}
			else if (const AuthoringVec2* v = std::get_if<AuthoringVec2>(&_value))
			{
				inst->parameters[name] = std::array<float, 2>{ static_cast<float>(v->x), static_cast<float>(v->y) };
			}
			else if (const AuthoringVec3* v = std::get_if<AuthoringVec3>(&_value))
			{
				inst->parameters[name] = std::array<float, 3>{
					static_cast<float>(v->x), static_cast<float>(v->y), static_cast<float>(v->z) };
			}
			else if (const AuthoringVec4* v = std::get_if<AuthoringVec4>(&_value))
			{
				inst->parameters[name] = std::array<float, 4>{
					static_cast<float>(v->x), static_cast<float>(v->y),
					static_cast<float>(v->z), static_cast<float>(v->w) };
			}
			else if (const std::string* pathStr = std::get_if<std::string>(&_value))
			{
				if (pathStr->empty())
				{
					inst->parameters[name] = AssetHandle<ITexture>{};
				}
				else
				{
					const auto resolved = ResolveAssetPath(*pathStr);
					auto handle = GetPluginAssetRegistry().Get<ITexture>().Load(resolved);
					if (!handle.IsValid())
					{
						_outError = "Failed to load texture: " + *pathStr;
						return false;
					}

					inst->parameters[name] = std::move(handle);
				}
			}
			else
			{
				_outError = "Unsupported value type for parameter: " + name;
				return false;
			}

			inst->InvalidateGpuBuffers();

			return MaterialInstanceSaver::Save(*inst, _assetPath);
		}

		_outError = "Unknown section: " + std::string(_sectionId);
		return false;
	}
}

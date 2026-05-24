#include "MaterialInstance.hpp"

#include <Common/IO/FileHelper.hpp>
#include <Resources/AssetRegistry.hpp>
#include "../ExamplePluginAssetRegistry.hpp"

#include <fstream>

namespace cp
{
	static constexpr uint32_t MatInstMagic = 0x4D494E53; // 'MINS'
	static constexpr uint32_t MatInstVersion = 1;

	static bool ReadString(std::ifstream& _ifs, std::string& _out)
	{
		uint32_t len = 0;
		_ifs.read(reinterpret_cast<char*>(&len), sizeof(len));
		_out.resize(len);

		if (len > 0)
		{
			_ifs.read(_out.data(), len);
		}

		return _ifs.good();
	}

	std::shared_ptr<MaterialInstance> AssetLoader<MaterialInstance>::Load(
		const std::filesystem::path& _path,
		RenderingHardwareInterface& _rhi
	)
	{
		std::ifstream ifs(_path, std::ios::binary);
		if (!ifs.is_open())
		{
			return nullptr;
		}

		uint32_t magic = 0, version = 0;
		ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic));
		ifs.read(reinterpret_cast<char*>(&version), sizeof(version));

		if (magic != MatInstMagic || version != MatInstVersion)
		{
			return nullptr;
		}

		std::string materialPath;
		if (!ReadString(ifs, materialPath))
		{
			return nullptr;
		}

		uint32_t paramCount = 0;
		ifs.read(reinterpret_cast<char*>(&paramCount), sizeof(paramCount));
		if (!ifs.good())
		{
			return nullptr;
		}

		std::unordered_map<std::string, ParameterValue> parameters;
		parameters.reserve(paramCount);

		for (uint32_t i = 0; i < paramCount; ++i)
		{
			std::string name;
			if (!ReadString(ifs, name))
			{
				return nullptr;
			}

			uint8_t typeTag = 0;
			ifs.read(reinterpret_cast<char*>(&typeTag), sizeof(typeTag));
			if (!ifs.good())
			{
				return nullptr;
			}

			switch (static_cast<ParameterValueType>(typeTag))
			{
				case ParameterValueType::Bool:
				{
					bool v = false;
					ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
					parameters[name] = v;
					break;
				}
				case ParameterValueType::Int32:
				{
					int32_t v = 0;
					ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
					parameters[name] = v;
					break;
				}
				case ParameterValueType::Float:
				{
					float v = 0.f;
					ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
					parameters[name] = v;
					break;
				}
				case ParameterValueType::Float2:
				{
					std::array<float, 2> v{};
					ifs.read(reinterpret_cast<char*>(v.data()), sizeof(v));
					parameters[name] = v;
					break;
				}
				case ParameterValueType::Float3:
				{
					std::array<float, 3> v{};
					ifs.read(reinterpret_cast<char*>(v.data()), sizeof(v));
					parameters[name] = v;
					break;
				}
				case ParameterValueType::Float4:
				{
					std::array<float, 4> v{};
					ifs.read(reinterpret_cast<char*>(v.data()), sizeof(v));
					parameters[name] = v;
					break;
				}
				case ParameterValueType::Texture2D:
				{
					std::string texturePath;
					if (!ReadString(ifs, texturePath))
					{
						return nullptr;
					}

					AssetHandle<ITexture> handle;
					if (!texturePath.empty())
					{
						const auto resolvedPath = ResolveAssetPath(texturePath);
						handle = GetPluginAssetRegistry().Get<ITexture>().Load(resolvedPath);
					}
					
					parameters[name] = std::move(handle);
					break;
				}
				default:
					return nullptr;
			}

			if (!ifs.good())
			{
				return nullptr;
			}
		}

		auto inst = std::make_shared<MaterialInstance>();
		inst->parameters = std::move(parameters);

		if (!materialPath.empty())
		{
			std::filesystem::path resolved;

			const std::filesystem::path asIs(materialPath);

			if (asIs.is_absolute() && std::filesystem::exists(asIs))
			{
				resolved = std::filesystem::weakly_canonical(asIs);
			}
			else
			{
				resolved = ResolveAssetPath(materialPath);

				if (resolved.empty())
				{
					const std::string filename = asIs.filename().string();

					if (!filename.empty())
					{
						resolved = ResolveAssetPath(filename);
					}
				}
			}

			if (!resolved.empty())
			{
				inst->material = GetPluginAssetRegistry().Get<Material>().Load(resolved);
			}
		}

		return inst;
	}
}

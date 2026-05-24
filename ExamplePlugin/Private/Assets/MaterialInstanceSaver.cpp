#include "MaterialInstanceSaver.hpp"

#include <Common/IO/FileHelper.hpp>

#include <fstream>

namespace cp
{
	static constexpr uint32_t MatInstMagic = 0x4D494E53;
	static constexpr uint32_t MatInstVersion = 1;

	static void WriteString(std::ofstream& ofs, const std::string& str)
	{
		const auto len = static_cast<uint32_t>(str.size());
		ofs.write(reinterpret_cast<const char*>(&len), sizeof(len));

		if (len > 0)
		{
			ofs.write(str.data(), len);
		}
	}

	bool MaterialInstanceSaver::Save(const MaterialInstance& instance, const std::filesystem::path& path)
	{
		std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
		if (!ofs.is_open())
		{
			return false;
		}

		ofs.write(reinterpret_cast<const char*>(&MatInstMagic), sizeof(MatInstMagic));
		ofs.write(reinterpret_cast<const char*>(&MatInstVersion), sizeof(MatInstVersion));

		std::string materialPath;
		if (instance.material.IsValid())
		{
			materialPath = GetRelativePath(instance.material.GetAssetID()).string();
		}

		WriteString(ofs, materialPath);

		const auto paramCount = static_cast<uint32_t>(instance.parameters.size());
		ofs.write(reinterpret_cast<const char*>(&paramCount), sizeof(paramCount));

		for (const auto& [name, value] : instance.parameters)
		{
			WriteString(ofs, name);

			std::visit([&](const auto& v)
			{
				using T = std::decay_t<decltype(v)>;

				if constexpr (std::is_same_v<T, bool>)
				{
					const auto tag = static_cast<uint8_t>(ParameterValueType::Bool);
					ofs.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
					ofs.write(reinterpret_cast<const char*>(&v), sizeof(v));
				}
				else if constexpr (std::is_same_v<T, int32_t>)
				{
					const auto tag = static_cast<uint8_t>(ParameterValueType::Int32);
					ofs.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
					ofs.write(reinterpret_cast<const char*>(&v), sizeof(v));
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					const auto tag = static_cast<uint8_t>(ParameterValueType::Float);
					ofs.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
					ofs.write(reinterpret_cast<const char*>(&v), sizeof(v));
				}
				else if constexpr (std::is_same_v<T, std::array<float, 2>>)
				{
					const auto tag = static_cast<uint8_t>(ParameterValueType::Float2);
					ofs.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
					ofs.write(reinterpret_cast<const char*>(v.data()), sizeof(v));
				}
				else if constexpr (std::is_same_v<T, std::array<float, 3>>)
				{
					const auto tag = static_cast<uint8_t>(ParameterValueType::Float3);
					ofs.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
					ofs.write(reinterpret_cast<const char*>(v.data()), sizeof(v));
				}
				else if constexpr (std::is_same_v<T, std::array<float, 4>>)
				{
					const auto tag = static_cast<uint8_t>(ParameterValueType::Float4);
					ofs.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
					ofs.write(reinterpret_cast<const char*>(v.data()), sizeof(v));
				}
				else if constexpr (std::is_same_v<T, AssetHandle<ITexture>>)
				{
					const auto tag = static_cast<uint8_t>(ParameterValueType::Texture2D);
					ofs.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
					const std::string texturePath = v.IsValid() ? GetRelativePath(v.GetAssetID()).string() : std::string{};
					WriteString(ofs, texturePath);
				}
			}, value);
		}

		return ofs.good();
	}
}

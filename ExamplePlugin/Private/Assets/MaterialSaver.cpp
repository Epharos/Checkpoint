#include "MaterialSaver.hpp"

#include <Common/IO/FileHelper.hpp>
#include "../ExamplePluginDir.hpp"

#include <fstream>

namespace cp
{
	static constexpr uint32_t MaterialMagic = 0x4D415400;
	static constexpr uint32_t MaterialVersion = 1;

	bool MaterialSaver::Save(const Material& material, const std::filesystem::path& path)
	{
		std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
		if (!ofs.is_open())
		{
			return false;
		}

		ofs.write(reinterpret_cast<const char*>(&MaterialMagic), sizeof(MaterialMagic));
		ofs.write(reinterpret_cast<const char*>(&MaterialVersion), sizeof(MaterialVersion));

		std::string shaderPathStr;
		if (!material.shaderPath.empty())
		{
			shaderPathStr = GetRelativePath(material.shaderPath, GetExamplePluginDir()).string();
		}

		const auto len = static_cast<uint32_t>(shaderPathStr.size());
		ofs.write(reinterpret_cast<const char*>(&len), sizeof(len));
		if (len > 0)
		{
			ofs.write(shaderPathStr.data(), len);
		}

		return ofs.good();
	}
}

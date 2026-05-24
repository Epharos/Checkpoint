#include "Material.hpp"

#include <Common/IO/FileHelper.hpp>
#include "../ExamplePluginDir.hpp"

#include <filesystem>
#include <fstream>

namespace cp
{
	static constexpr uint32_t MaterialMagic = 0x4D415400; // 'MAT\0'
	static constexpr uint32_t MaterialVersion = 1;

	std::shared_ptr<Material> AssetLoader<Material>::Load(
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

		if (magic != MaterialMagic || version != MaterialVersion)
		{
			return nullptr;
		}

		uint32_t pathLen = 0;
		ifs.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen));

		std::string shaderPath(pathLen, '\0');
		if (pathLen > 0)
		{
			ifs.read(shaderPath.data(), pathLen);
		}

		if (!ifs.good())
		{
			return nullptr;
		}

		auto mat = std::make_shared<Material>();

		if (!shaderPath.empty())
		{
			const std::filesystem::path asIs(shaderPath);

			if (asIs.is_absolute() && std::filesystem::exists(asIs))
			{
				mat->shaderPath = std::filesystem::weakly_canonical(asIs);
			}
			else
			{
				const auto asPluginRelative = FindFileFromDirectory(shaderPath, GetExamplePluginDir());

				if (!asPluginRelative.empty())
				{
					mat->shaderPath = asPluginRelative;
				}
				else
				{
					const auto asProjectRelative = ResolveAssetPath(shaderPath);

					if (!asProjectRelative.empty())
					{
						mat->shaderPath = asProjectRelative;
					}
					else
					{
						const std::string filename = asIs.filename().string();

						mat->shaderPath = filename.empty() ? asIs : FindFileFromDirectory(filename, GetExamplePluginDir());
					}
				}
			}
		}

		return mat;
	}
}

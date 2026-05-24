#pragma once

#include "MaterialSaver.hpp"

#include <Common/Plugin/AssetContextMenu.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace cp
{
	class SlangContextMenuContributor final : public IAssetContextMenuContributor
	{
	public:
		[[nodiscard]] std::string_view FileExtension() const override
		{
			return "slang";
		}

		[[nodiscard]] std::vector<AssetContextMenuAction> GetActions(
			const std::filesystem::path& assetPath
		) const override
		{
			return {
				{
					.label = "Create Material (.material)",
					.callback = [assetPath]()
					{
						std::filesystem::path dest = assetPath;
						dest.replace_extension(".material");

						if (std::filesystem::exists(dest))
						{
							return;
						}

						Material mat;
						mat.shaderPath = assetPath;
						MaterialSaver::Save(mat, dest);
					}
				}
			};
		}
	};
}

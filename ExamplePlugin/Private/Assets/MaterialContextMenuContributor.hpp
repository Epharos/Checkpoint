#pragma once

#include "MaterialInstance.hpp"
#include "MaterialInstanceSaver.hpp"

#include <Common/Plugin/AssetContextMenu.hpp>
#include <Resources/AssetRegistry.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace cp
{
	class MaterialContextMenuContributor final : public IAssetContextMenuContributor
	{
	public:
		explicit MaterialContextMenuContributor(AssetRegistry& assetRegistry)
			: assetRegistry(assetRegistry)
		{
		}

		[[nodiscard]] std::string_view FileExtension() const override
		{
			return "material";
		}

		[[nodiscard]] std::vector<AssetContextMenuAction> GetActions(
			const std::filesystem::path& _assetPath
		) const override
		{
			return {
				{
					.label = "Create MaterialInstance (.matinst)",
					.callback = [this, _assetPath]()
					{
						std::filesystem::path dest = _assetPath;
						dest.replace_extension(".matinst");
						if (std::filesystem::exists(dest))
						{
							return;
						}
						MaterialInstance inst;
						inst.material = assetRegistry.Get<Material>().Load(_assetPath);
						MaterialInstanceSaver::Save(inst, dest);
					}
				}
			};
		}

	private:
		AssetRegistry& assetRegistry;
	};
}

#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace cp
{
	struct AssetContextMenuAction
	{
		std::string label;
		std::function<void()> callback;
	};

	class IAssetContextMenuContributor
	{
	public:
		virtual ~IAssetContextMenuContributor() = default;

		[[nodiscard]] virtual std::string_view FileExtension() const = 0;
		[[nodiscard]] virtual std::vector<AssetContextMenuAction> GetActions(
			const std::filesystem::path& assetPath
		) const = 0;
	};
}

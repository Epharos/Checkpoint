#pragma once

#include "AssetContextMenu.hpp"

#include <memory>
#include <string_view>

namespace cp
{
	class IAssetContextMenuRegistrar
	{
	public:
		virtual ~IAssetContextMenuRegistrar() = default;

		virtual void Register(std::shared_ptr<IAssetContextMenuContributor> contributor) = 0;
		virtual void Unregister(std::string_view extension) = 0;
	};
}

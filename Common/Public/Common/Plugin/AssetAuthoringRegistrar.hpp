#pragma once

#include "AssetAuthoring.hpp"

#include <memory>
#include <string_view>

namespace cp
{
	class IAssetAuthoringRegistrar
	{
	public:
		virtual ~IAssetAuthoringRegistrar() = default;

		virtual void Register(std::shared_ptr<IAssetAuthoring> authoring) = 0;
		virtual void Unregister(std::string_view extension) = 0;
	};
}

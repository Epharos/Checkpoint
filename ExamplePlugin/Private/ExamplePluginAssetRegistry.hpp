#pragma once

namespace cp
{
	class AssetRegistry;

	inline AssetRegistry* g_pluginAssetRegistry = nullptr;

	inline void SetPluginAssetRegistry(AssetRegistry* registry)
	{
		g_pluginAssetRegistry = registry;
	}

	inline AssetRegistry& GetPluginAssetRegistry()
	{
		return *g_pluginAssetRegistry;
	}
}

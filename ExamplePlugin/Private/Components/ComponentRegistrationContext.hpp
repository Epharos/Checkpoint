#pragma once

#include <Resources/AssetRegistry.hpp>
#include <stdexcept>

namespace cp
{
	class ComponentRegistrationContext
	{
	private:
		AssetRegistry* assetRegistry;

	public:
		explicit ComponentRegistrationContext(AssetRegistry* assetRegistry)
			: assetRegistry(assetRegistry)
		{
			if (!assetRegistry)
				throw std::runtime_error("AssetRegistry cannot be null");
		}

		[[nodiscard]] AssetRegistry& GetAssetRegistry() const
		{
			return *assetRegistry;
		}
	};

	inline thread_local ComponentRegistrationContext* componentRegistrationContext = nullptr;

	inline void SetComponentRegistrationContext(ComponentRegistrationContext* context)
	{
		componentRegistrationContext = context;
	}

	inline ComponentRegistrationContext& GetComponentRegistrationContext()
	{
		if (!componentRegistrationContext)
		{
			throw std::runtime_error("Component registration context not configured.");
		}
		return *componentRegistrationContext;
	}
}

#pragma once

#include <memory>
#include <string>

namespace cp
{
	template<typename TResource>
	class AssetHandle
	{
	public:
		AssetHandle() = default;

		AssetHandle(std::shared_ptr<TResource> _resource, std::string _assetID)
			: resource(std::move(_resource))
			, assetID(std::move(_assetID))
		{
		}

		TResource* Get() const { return resource.get(); }
		TResource* operator->() const { return resource.get(); }
		TResource& operator*() const { return *resource; }

		[[nodiscard]] bool IsValid() const { return resource != nullptr; }
		explicit operator bool() const { return IsValid(); }

		[[nodiscard]] const std::string& GetAssetID() const { return assetID; }

		std::shared_ptr<TResource> GetSharedPtr() const { return resource; }

	private:
		std::shared_ptr<TResource> resource;
		std::string assetID;
	};
}

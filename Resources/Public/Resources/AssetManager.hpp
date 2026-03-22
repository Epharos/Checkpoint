#pragma once

#include "AssetHandle.hpp"
#include "AssetLoader.hpp"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace cp
{
	class RenderingHardwareInterface;

	template<typename TResource>
	class AssetManager
	{
	public:
		explicit AssetManager(RenderingHardwareInterface& _rhi)
			: rhi(_rhi)
		{
		}

		AssetHandle<TResource> Load(const std::filesystem::path& _path)
		{
			std::lock_guard<std::mutex> lock(mutex);

			auto it = cache.find(_path.string());
			if (it != cache.end())
			{
				if (auto existing = it->second.lock())
				{
					return AssetHandle<TResource>(existing, _path.string());
				}

				cache.erase(it);
			}

			return LoadAndCache(_path);
		}

		AssetHandle<TResource> Reload(const std::filesystem::path& _path)
		{
			std::lock_guard<std::mutex> lock(mutex);

			auto it = cache.find(_path.string());
			if (it != cache.end())
			{
				cache.erase(it);
			}

			return LoadAndCache(_path);
		}

		size_t GetLoadedCount() const
		{
			std::lock_guard<std::mutex> lock(mutex);
			return cache.size();
		}

		size_t GetActiveCount() const
		{
			std::lock_guard<std::mutex> lock(mutex);
			
			size_t count = 0;
			for (const auto& [_, weakPtr] : cache)
			{
				if (!weakPtr.expired())
				{
					++count;
				}
			}

			return count;
		}

		void PurgeExpired()
		{
			std::lock_guard<std::mutex> lock(mutex);

			for (auto it = cache.begin(); it != cache.end();)
			{
				if (it->second.expired())
				{
					it = cache.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

	private:
		AssetHandle<TResource> LoadAndCache(const std::filesystem::path& _path)
		{
			auto resource = AssetLoader<TResource>::Load(_path, rhi);
			if (!resource)
			{
				return AssetHandle<TResource>();
			}

			cache[_path] = resource;
			return AssetHandle<TResource>(resource, _path.string());
		}

		RenderingHardwareInterface& rhi;
		std::unordered_map<std::filesystem::path, std::weak_ptr<TResource>> cache;
		mutable std::mutex mutex;
	};
}

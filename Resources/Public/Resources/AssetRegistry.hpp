#pragma once

#include "AssetManager.hpp"

#include <any>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>

namespace cp
{
	class AssetRegistry
	{
	public:
		AssetRegistry(const AssetRegistry&) = delete;
		AssetRegistry& operator=(const AssetRegistry&) = delete;
		AssetRegistry(const AssetRegistry&&) = delete;
		AssetRegistry& operator=(const AssetRegistry&&) = delete;

		static AssetRegistry& Instance()
		{
			static AssetRegistry instance;
			return instance;
		}

		void Initialize(RenderingHardwareInterface& _rhi)
		{
			std::lock_guard<std::mutex> lock(mutex);
			rhi = &_rhi;
		}

		template<typename TResource>
		AssetManager<TResource>& Get()
		{
			std::lock_guard<std::mutex> lock(mutex);

			if (!rhi)
			{
				throw std::runtime_error("AssetRegistry not initialized. Call Initialize() first.");
			}

			const auto typeIndex = std::type_index(typeid(TResource));

			const auto it = managers.find(typeIndex);
			if (it == managers.end())
			{
				auto manager = std::make_shared<AssetManager<TResource>>(*rhi);
				managers[typeIndex] = manager;
				return *manager;
			}

			return *std::any_cast<std::shared_ptr<AssetManager<TResource>>>(it->second);
		}

		void Cleanup()
		{
			std::lock_guard<std::mutex> lock(mutex);

			managers.clear();
		}

	private:
		AssetRegistry() = default;
		~AssetRegistry() = default;

		RenderingHardwareInterface* rhi = nullptr;
		std::unordered_map<std::type_index, std::any> managers;
		mutable std::mutex mutex;
	};
}

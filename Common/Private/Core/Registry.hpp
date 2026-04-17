#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cp
{
	template<typename T>
	class Registry final
	{
	public:
		using BaseType = T;
		using Creator = std::function<std::unique_ptr<T>()>;

		bool Register(std::string _name, Creator _creator)
		{
			if (_name.empty())
			{
				throw std::invalid_argument("Empty type name when registering " + _name + ".");
			}

			if (!_creator)
			{
				throw std::invalid_argument("Invalid creator for type '" + _name + "'.");
			}

			std::lock_guard lock(mutex);
			if (!entries.contains(_name))
			{
				entries.emplace(std::move(_name), std::move(_creator));
				return true;
			}

			return false;
		}

		template<std::derived_from<T> TImpl>
		bool RegisterType(std::string _name)
		{
			return Register(std::move(_name), []() { return std::make_unique<TImpl>(); });
		}

		bool Unregister(const std::string_view _name)
		{
			std::lock_guard lock(mutex);
			if (const auto it = entries.find(std::string(_name)); it != entries.end())
			{
				entries.erase(it);
				return true;
			}

			return false;
		}

		[[nodiscard]] bool Contains(const std::string_view _name) const
		{
			std::lock_guard lock(mutex);
			return entries.contains(std::string(_name));
		}

		[[nodiscard]] std::vector<std::string> Names() const
		{
			std::lock_guard lock(mutex);
			std::vector<std::string> names;
			names.reserve(entries.size());

			for (const auto& [name, _] : entries)
			{
				names.push_back(name);
			}

			std::sort(names.begin(), names.end());

			return names;
		}

		[[nodiscard]] std::unique_ptr<T> Create(const std::string_view _name) const
		{
			Creator creator;

			{
				std::lock_guard lock(mutex);
				const auto it = entries.find(std::string(_name));

				if (it == entries.end())
				{
					throw std::invalid_argument("Unknown type name '" + std::string(_name) + "'.");
				}

				creator = it->second;
			}

			return creator();
		}

		void Clear()
		{
			std::lock_guard lock(mutex);
			entries.clear();
		}

	private:
		using RegistryMap = std::unordered_map<std::string, Creator>;
		RegistryMap entries;
		mutable std::mutex mutex;
	};

	class RegistryManager final
	{
	public:
		template<typename T>
		Registry<T>& GetOrCreate(std::string _registryName)
		{
			if (_registryName.empty())
			{
				throw std::invalid_argument("Empty registry name.");
			}

			std::lock_guard lock(mutex);
			const auto it = registries.find(_registryName);
			if (it == registries.end())
			{
				auto holder = std::make_unique<TypedRegistryHolder<T>>();
				Registry<T>& registry = holder->registry;
				registries.emplace(std::move(_registryName), std::move(holder));
				return registry;
			}

			return GetTypedRegistry<T>(_registryName, *it->second);
		}

		template<typename T>
		[[nodiscard]] Registry<T>* Find(std::string_view _registryName)
		{
			if (_registryName.empty())
			{
				return nullptr;
			}

			std::lock_guard lock(mutex);
			const auto it = registries.find(std::string(_registryName));
			if (it == registries.end())
			{
				return nullptr;
			}

			if (it->second->type != std::type_index(typeid(T)))
			{
				return nullptr;
			}

			return &static_cast<TypedRegistryHolder<T>*>(it->second.get())->registry;
		}

		template<typename T>
		[[nodiscard]] const Registry<T>* Find(std::string_view _registryName) const
		{
			return const_cast<RegistryManager*>(this)->Find<T>(_registryName);
		}

		[[nodiscard]] bool Contains(std::string_view _registryName) const
		{
			if (_registryName.empty())
			{
				return false;
			}

			std::lock_guard lock(mutex);
			return registries.contains(std::string(_registryName));
		}

		bool Remove(std::string_view _registryName)
		{
			if (_registryName.empty())
			{
				return false;
			}

			std::lock_guard lock(mutex);
			return registries.erase(std::string(_registryName)) > 0;
		}

		void Clear()
		{
			std::lock_guard lock(mutex);
			registries.clear();
		}

		[[nodiscard]] std::vector<std::string> Names() const
		{
			std::lock_guard lock(mutex);
			std::vector<std::string> names;
			names.reserve(registries.size());

			for (const auto& [name, _] : registries)
			{
				names.push_back(name);
			}

			std::sort(names.begin(), names.end());
			return names;
		}

	private:
		struct RegistryHolder
		{
			explicit RegistryHolder(std::type_index _type) : type(_type)
			{
			}

			virtual ~RegistryHolder() = default;
			std::type_index type;
		};

		template<typename T>
		struct TypedRegistryHolder final : RegistryHolder
		{
			TypedRegistryHolder() : RegistryHolder(std::type_index(typeid(T)))
			{
			}

			Registry<T> registry;
		};

		template<typename T>
		[[nodiscard]] static Registry<T>& GetTypedRegistry(const std::string& _name, RegistryHolder& _holder)
		{
			const auto requestedType = std::type_index(typeid(T));
			if (_holder.type != requestedType)
			{
				throw std::logic_error(
					"Registry '" + _name + "' already exists with a different value type."
				);
			}

			return static_cast<TypedRegistryHolder<T>&>(_holder).registry;
		}

		std::unordered_map<std::string, std::unique_ptr<RegistryHolder>> registries;
		mutable std::mutex mutex;
	};
}

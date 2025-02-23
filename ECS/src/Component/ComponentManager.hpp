#pragma once

#include "../pch.hpp"
#include "../Entity.hpp"
#include "ComponentSparseSet.hpp"
#include "ComponentBase.hpp"

namespace ECS
{
	class ComponentManager
	{
	private:
		std::unordered_map<std::type_index, std::unique_ptr<SparseSet>> componentStorage;

		template<typename T>
		ComponentSparseSet<T>& GetOrCreateComponentSparseSet()
		{
			std::type_index type = std::type_index(typeid(T));
			if (componentStorage.find(type) == componentStorage.end())
			{
				componentStorage[type] = std::make_unique<ComponentSparseSet<T>>();
			}

			return *static_cast<ComponentSparseSet<T>*>(componentStorage[type].get());
		}

	public:

		template<typename T>
		bool AddComponent(Entity entity, T component)
		{
			if (!std::is_base_of<IComponentBase, T>::value)
				throw std::runtime_error("Component must inherit from IComponentBase");

			return GetOrCreateComponentSparseSet<T>().Add(std::move(entity), std::move(component));
		}

		template<typename T>
		bool RemoveComponent(Entity entity)
		{
			return GetOrCreateComponentSparseSet<T>().Remove(std::move(entity));
		}

		template<typename T>
		T& GetComponent(Entity entity)
		{
			return GetOrCreateComponentSparseSet<T>().Get(std::move(entity));
		}

		template<typename ...T>
		std::tuple<T&...> GetComponents(Entity entity)
		{
			return std::tuple<T&...>{GetComponent<T>(entity)...};
		}

		std::vector<std::pair<std::type_index, void*>> GetAllComponentsOf(Entity entity)
		{
			std::vector<std::pair<std::type_index, void*>> result;

			for (auto& [type, storage] : componentStorage)
			{
				if (storage->Has(entity))
				{
					result.push_back({ type, storage->GetRaw(entity) });
				}
			}

			return result;
		}

		template<typename T>
		bool HasComponent(Entity entity) const
		{
			auto it = componentStorage.find(std::type_index(typeid(T)));
			if (it == componentStorage.end()) return false;
			return static_cast<ComponentSparseSet<T>*>(it->second.get())->Has(std::move(entity));
		}

		template<typename T>
		void ForEachComponent(std::function<void(Entity, T&)> func)
		{
			GetOrCreateComponentSparseSet<T>().ForEach(func);
		}

		template<typename MainComponent, typename ...Others>
		void ForEachArchetype(std::function<void(Entity entity, MainComponent& mainComponent, Others&... others)> func)
		{
			ForEachComponent<MainComponent>([&](Entity entity, MainComponent& mainComponent)
				{
					if ((HasComponent<Others>(entity) && ...))
						func(entity, mainComponent, GetComponent<Others>(entity)...);
				});
		}

		template<typename Component>
		Entity FindFirstWith()
		{
			auto entities = GetOrCreateComponentSparseSet<Component>().GetEntities();
			if (entities.empty()) return EntityManager::NULL_ENTITY;
			return entities.front();
		}

		template<typename Component>
		std::vector<Entity> FindAllWith()
		{
			return GetOrCreateComponentSparseSet<Component>().GetEntities();
		}

		template<typename MainComponent, typename ...Others>
		std::vector<std::tuple<MainComponent&, Others&...>> QueryArchetype()
		{
			std::vector<std::tuple<MainComponent&, Others&...>> result;

			auto entities = GetOrCreateComponentSparseSet<MainComponent>().GetEntities();

			for (auto entity : entities)
			{
				if ((HasComponent<Others>(entity) && ...))
				{
					std::tuple<MainComponent&, Others&...> tuple(GetComponent<MainComponent>(entity), GetComponent<Others>(entity)...);
					result.push_back(tuple);
				}
			}

			return result;
		}


		void RemoveAllComponents(Entity entity)
		{
			for (auto& [type, storage] : componentStorage)
			{
				storage->Remove(std::move(entity));
			}
		}
	};
}
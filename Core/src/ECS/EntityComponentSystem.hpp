#pragma once

#include "../pch.hpp"
#include "Entity/EntityManager.hpp"
#include "Component/ComponentManager.hpp"
#include "System/SystemManager.hpp"

namespace cp
{
	class EntityComponentSystem
	{
	private:
		EntityManager entityManager;
		ComponentManager componentManager;
		SystemManager systemManager;

	public:
		Entity CreateEntity()
		{
			return entityManager.CreateEntity();
		}

		template <typename T>
		bool AddComponent(Entity _entity, T _component)
		{
			return componentManager.AddComponent<T>(_entity, _component);
		}

		template <typename T>
		bool AddComponent(Entity _entity)
		{
			return componentManager.AddComponent<T>(_entity, {});
		}

		std::list<Entity> GetEntities() const
		{
			return entityManager.GetEntities();
		}

		template <typename T>
		bool RemoveComponent(Entity _entity)
		{
			return componentManager.RemoveComponent<T>(_entity);
		}

		template <typename T>
		T& GetComponent(Entity _entity)
		{
			return componentManager.GetComponent<T>(_entity);
		}

		/*
		* @brief Get component of a specific type from an entity
		* Should not be used too often as it returns a void pointer (which create indirections)
		* Useful when you need to get a component of an unknown type (eg. for deserialization)
		*/
		void* GetComponent(Entity _entity, std::type_index _type)
		{
			return componentManager.GetComponent(_entity, _type);
		}

		void* GetComponent(Entity _entity, const std::string& _typeName)
		{
			return componentManager.GetComponent(_entity, _typeName);
		}

		template <typename ...T>
		std::tuple<T ...> GetComponents(Entity _entity)
		{
			return componentManager.GetComponents<T...>(_entity);
		}

		std::vector<std::pair<std::type_index, void*>> GetAllComponentsOf(Entity _entity)
		{
			return componentManager.GetAllComponentsOf(_entity);
		}

		const std::vector<std::pair<std::type_index, void*>> GetAllComponentsOf(Entity _entity) const
		{
			return componentManager.GetAllComponentsOf(_entity);
		}

		template <typename T, typename... Args>
		T& RegisterSystem(Args&& ... _args)
		{
			T& system = systemManager.RegisterSystem<T>(std::forward<Args>(_args)...);
			system.OnRegister(entityManager, componentManager);
			return system;
		}

		void Update(const float& _dt)
		{
			systemManager.Update(entityManager, componentManager, _dt);
		}

		void DestroyEntity(Entity _entity)
		{
			entityManager.DestroyEntity(_entity);
			componentManager.RemoveAllComponents(_entity);
		}

		bool IsValid(Entity _entity) const
		{
			return entityManager.IsValid(_entity);
		}

		template <typename T>
		bool HasComponent(Entity _entity) const
		{
			return componentManager.HasComponent<T>(_entity);
		}

		template <typename T>
		void ForEachComponent(std::function<void(Entity, T&)> _func)
		{
			componentManager.ForEachComponent<T>(_func);
		}

		uint32_t GetEntityCount() const
		{
			return entityManager.GetEntityCount();
		}

		uint8_t GetValidVersion(Entity _entity) const
		{
			return entityManager.GetValidVersion(_entity);
		}

		void Cleanup()
		{
			systemManager.Cleanup();
		}
	};
}
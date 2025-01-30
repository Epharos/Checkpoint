#pragma once

#include "pch.hpp"
#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"

namespace ECS
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

		template <typename ...T>
		std::tuple<T ...> GetComponents(Entity _entity)
		{
			return componentManager.GetComponents<T...>(_entity);
		}

		template <typename T, typename... Args>
		T& RegisterSystem(Args&& ... _args)
		{
			T& system = systemManager.RegisterSystem<T>(_args...);
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
	};
}
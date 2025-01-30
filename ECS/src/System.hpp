#pragma once

#include "ComponentManager.hpp"

namespace ECS
{
	class System
	{
	public:
		virtual void OnRegister(EntityManager& _entityManager, ComponentManager& _componentManager) {};
		virtual void Update(EntityManager& _entityManager, ComponentManager& _componentManager, const float& _dt) = 0;
		virtual void Cleanup() = 0;
	};
}
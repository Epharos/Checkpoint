#pragma once

#include "pch.hpp"

#include "../Entity/EntityManager.hpp"
#include "../Component/ComponentManager.hpp"

namespace cp
{
	class System
	{
	public:
		virtual ~System() = default;
		virtual void OnRegister(EntityManager& _entityManager, ComponentManager& _componentManager) {};
		virtual void Update(EntityManager& _entityManager, ComponentManager& _componentManager, const float& _dt) = 0;
		virtual void Cleanup() = 0;
	};
}
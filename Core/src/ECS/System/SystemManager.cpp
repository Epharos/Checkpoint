#include "pch.hpp"

#include "SystemManager.hpp"

namespace cp
{
	void SystemManager::Update(EntityManager& _entityManager, ComponentManager& _componentManager, const float& _dt)
	{
		for (auto& system : systems)
		{
			system->Update(_entityManager, _componentManager, _dt);
		}
	}

	void SystemManager::Cleanup()
	{
		for (auto& system : systems)
		{
			system->Cleanup();
		}
	}
}
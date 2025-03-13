#include "pch.hpp"
#include "EntityManager.hpp"

namespace ECS
{
	EntityManager::EntityManager()
	{
		nextID = 0;
	}

	Entity EntityManager::CreateEntity()
	{
		if (availableIDs.empty())
		{
			ID entityID = nextID++;
			versions.push_back(0);

#ifdef IN_EDITOR
			entities.push_back({ entityID, 0 });
#endif

			return { entityID, 0 };
		}

		ID entityID = availableIDs.back();
		availableIDs.pop_back();

#ifdef IN_EDITOR
		entities.push_back({ entityID, 0 });
#endif

		return { entityID, versions[entityID] };
	}

	void EntityManager::DestroyEntity(ID _entityID)
	{
		versions[_entityID]++;
		availableIDs.push_back(_entityID);

#ifdef IN_EDITOR		
		std::erase_if(entities, [_entityID](const Entity& _entity) { return _entity.id == _entityID; });
#endif
	}

	void EntityManager::DestroyEntity(Entity _entity)
	{
		DestroyEntity(_entity.id);
	}

	bool EntityManager::IsValid(Entity _entity) const
	{
		return _entity.version == versions[_entity.id];
	}

	uint8_t EntityManager::GetValidVersion(Entity _entity) const
	{
		return versions[_entity.id];
	}

	uint32_t EntityManager::GetEntityCount() const
	{
		return nextID - static_cast<uint32_t>(availableIDs.size());
	}
}
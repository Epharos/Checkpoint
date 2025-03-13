#pragma once

#include "../../pch.hpp"
#include "Entity.hpp"

namespace ECS
{
	class EntityManager
	{
	private:
		ID nextID;
		std::deque<ID> availableIDs;
		std::vector<Version> versions;

#ifdef IN_EDITOR
		std::list<Entity> entities;
#endif

	public:
		inline static Entity NULL_ENTITY = { static_cast<ID>(-1), 0};
		EntityManager();
		Entity CreateEntity();

		void DestroyEntity(ID _entityID);
		void DestroyEntity(Entity _entity);

		bool IsValid(Entity _entity) const;

		uint8_t GetValidVersion(Entity _entity) const;
		uint32_t GetEntityCount() const;

#ifdef IN_EDITOR
		inline const std::list<Entity>& GetEntities() const { return entities; }
#endif
	};
}
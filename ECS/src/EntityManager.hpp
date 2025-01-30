#pragma once

#include "pch.hpp"

namespace ECS
{
	class EntityManager
	{
	private:
		ID nextID;
		std::deque<ID> availableIDs;
		std::vector<Version> versions;

		

	public:
		inline static Entity NULL_ENTITY = { -1, 0 };
		EntityManager();
		Entity CreateEntity();

		void DestroyEntity(ID _entityID);
		void DestroyEntity(Entity _entity);

		bool IsValid(Entity _entity) const;

		uint8_t GetValidVersion(Entity _entity) const;
		uint32_t GetEntityCount() const;
	};
}
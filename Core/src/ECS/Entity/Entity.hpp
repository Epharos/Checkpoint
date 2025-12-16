#pragma once

#include "pch.hpp"
#include "../../Util/Serializers/ISerializer.hpp"

using ID = uint32_t;
using Version = uint32_t;

namespace cp
{
	class EntityComponentSystem;

	struct Entity
	{
		ID id : 24;
		Version version : 8;

		Entity(ID _id, Version _version) : id(_id), version(_version) {}

		bool operator==(const Entity& _other) const
		{
			return id == _other.id && version == _other.version;
		}

		bool operator!=(const Entity& _other) const
		{
			return !(*this == _other);
		}

		static void Serialize(const Entity& _entity, const std::vector<std::pair<std::type_index, void*>>& _components, cp::ISerializer& _serializer);
		static void Deserialize(Entity& _entity, cp::EntityComponentSystem& _ecs, cp::ISerializer& _serializer);
	};
}
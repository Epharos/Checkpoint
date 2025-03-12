#pragma once

#include "pch.hpp"
#include "../../Util/Serializers/Serializer.hpp"
#include "../Component/ComponentBase.hpp"
#include "../Component/ComponentRegistry.hpp"

using ID = uint32_t;
using Version = uint8_t;

struct Entity
{
	ID id : 24;
	Version version : 8;

	Entity(ID _id, Version _version) : id(_id), version(_version)
	{
		#ifdef IN_EDITOR
		displayName = "Entity " + std::to_string(id);
		#endif
	}

	bool operator==(const Entity& _other) const
	{
		return id == _other.id && version == _other.version;
	}

	bool operator!=(const Entity& _other) const
	{
		return !(*this == _other);
	}

	static void Serialize(const Entity& _entity, const std::vector<std::pair<std::type_index, void*>>& _components, Serializer& _serializer)
	{
		_serializer.WriteInt("ID", _entity.id);

		const void** components = new const void* [_components.size()];
		for (size_t i = 0; i < _components.size(); i++)
		{
			components[i] = _components[i].second;
		}

		_serializer.WriteObjectArray("Components", _components.size(), components, [&_serializer](const void* _component, Serializer& _s)
			{
				ComponentRegistry::GetInstance().CreateSerializer(*dynamic_cast<const IComponentBase*>(_component))->Serialize(_s);
			});
	}


#ifdef IN_EDITOR
	inline constexpr const std::string GetDisplayName() const { return displayName; }
	inline constexpr const void SetDisplayName(const std::string& _displayName) { displayName = _displayName; }
#endif

protected:
#ifdef IN_EDITOR
	std::string displayName;
#endif
};
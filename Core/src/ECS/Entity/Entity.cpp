#include "pch.hpp"

#include "Entity.hpp"

#include "../Component/ComponentRegistry.hpp"

void Entity::Serialize(const Entity& _entity, const std::vector<std::pair<std::type_index, void*>>& _components, ISerializer& _serializer)
{
	_serializer.WriteInt("ID", _entity.id);

	_serializer.BeginObjectArray("Components");

	for (const auto& component : _components)
	{
		_serializer.BeginObjectArrayElementWriting();
		_serializer.WriteString("Type", ComponentRegistry::GetInstance().GetTypeIndexMap().at(component.first));
		IComponentBase* componentBase = static_cast<IComponentBase*>(component.second);
		ISerializable* componentSerializer = ComponentRegistry::GetInstance().CreateSerializer(component.first, componentBase).release();
		_serializer.BeginObjectWriting("Data");
		componentSerializer->Serialize(_serializer);
		_serializer.EndObject();
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

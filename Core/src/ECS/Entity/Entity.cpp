#include "pch.hpp"

#include "Entity.hpp"

#include "../Component/ComponentRegistry.hpp"

void Entity::Serialize(const Entity& _entity, const std::vector<std::pair<std::type_index, void*>>& _components, Serializer& _serializer)
{
		_serializer.WriteInt("ID", _entity.id);

		_serializer.BeginObjectArray("Components");

		for (const auto& component : _components)
		{
			_serializer.BeginObjectArrayElement();
			_serializer.WriteString("Type", ComponentRegistry::GetInstance().GetTypeIndexMap().at(component.first));
			const Serializable* componentSerializer = ComponentRegistry::GetInstance().CreateSerializer(*static_cast<IComponentBase*>(component.second)).release();
			_serializer.BeginObject("Data");
			componentSerializer->Serialize(_serializer);
			_serializer.EndObject();
			_serializer.EndObjectArrayElement();
		}

		_serializer.EndObjectArray();
}

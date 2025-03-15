#include "pch.hpp"

#include "Entity.hpp"

#include "../Component/ComponentRegistry.hpp"

#include "Util/Serializers/ISerializer.hpp"

void cp::Entity::Serialize(const Entity& _entity, const std::vector<std::pair<std::type_index, void*>>& _components, ISerializer& _serializer)
{
	_serializer.WriteInt("ID", _entity.id);

	_serializer.BeginObjectArrayWriting("Components");

	for (const auto& component : _components)
	{
		_serializer.BeginObjectArrayElementWriting();

		_serializer.WriteString("Type", cp::ComponentRegistry::GetInstance().GetTypeIndexMap().at(component.first));
		cp::IComponentBase* componentBase = static_cast<cp::IComponentBase*>(component.second);
		cp::ISerializable* componentSerializer = cp::ComponentRegistry::GetInstance().CreateSerializer(component.first, componentBase).release();
		_serializer.BeginObjectWriting("Data");
		componentSerializer->Serialize(_serializer);

		_serializer.EndObject();
		_serializer.EndObjectArrayElement();

		delete componentSerializer;
	}

	_serializer.EndObjectArray();
}

void cp::Entity::Deserialize(Entity& _entity, EntityComponentSystem& _ecs, ISerializer& _serializer)
{
	uint64_t elements = -1;

	if ((elements = _serializer.BeginObjectArrayReading("Components")) > 0)
	{
		for (uint64_t index = 0; index < elements; index++)
		{
			if (!_serializer.BeginObjectArrayElementReading(index)) continue;

			std::string componentTypeStr = _serializer.ReadString("Type", "No String");

			if (!ComponentRegistry::GetInstance().CreateComponent(_ecs, _entity, componentTypeStr))
			{
				throw std::runtime_error("Couldn't deserialize component");
			}
			
			_serializer.BeginObjectReading("Data");
			std::type_index componentType = ComponentRegistry::GetInstance().GetTypeIndex(componentTypeStr);
			IComponentBase* component = static_cast<IComponentBase*>(_ecs.GetComponent(_entity, componentType));
			ISerializable* componentSerializer = ComponentRegistry::GetInstance().CreateSerializer(componentType, component).release();
			componentSerializer->Deserialize(_serializer);
			_serializer.EndObject();

			_serializer.EndObjectArrayElement();

			delete componentSerializer;
		}

		_serializer.EndObjectArray();
	}
}

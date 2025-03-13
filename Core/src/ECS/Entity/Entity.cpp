#include "pch.hpp"

#include "Entity.hpp"

#include "../Component/ComponentRegistry.hpp"

void Entity::Serialize(const Entity& _entity, const std::vector<std::pair<std::type_index, void*>>& _components, Serializer& _serializer)
{
		_serializer.WriteInt("ID", _entity.id);

		const void** components = new const void* [_components.size()];
		for (size_t i = 0; i < _components.size(); i++)
		{
			components[i] = _components[i].second;
		}

		_serializer.WriteObjectArray("Components", _components.size(), components, [&_serializer](const void* _component, Serializer& _s)
			{
				const IComponentBase* component = static_cast<const IComponentBase*>(_component);
				_s.WriteString("Type", ComponentRegistry::GetInstance().GetTypeIndexMap().at(typeid(*component)));
				const Serializable* componentSerializer = ComponentRegistry::GetInstance().CreateSerializer(*component).release();
				_s.WriteObject("Data", componentSerializer);
			});
}

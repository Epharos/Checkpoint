#include "ECSWrapper.hpp"

void cp::EntityAsset::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Entity Name", name);

	//TODO : Serialize Components
}

void cp::EntityAsset::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Entity Name", "Entity");
}

void cp::SceneAsset::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Scene Name", name);

	_serializer.BeginObjectArrayWriting("Entities");

	for (auto& entity : entities) {
		_serializer.BeginObjectArrayElementWriting();
		entity->Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

void cp::SceneAsset::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Scene Name", "Unnamed scene");

	if (size_t elements = _serializer.BeginObjectArrayReading("Entities")) {
		for (uint64_t index = 0; index < elements; index++) {
			if (!_serializer.BeginObjectArrayElementReading(index)) continue;

			EntityAsset* entity = new EntityAsset();
			entity->Deserialize(_serializer);
			entities.push_back(std::move(entity));

			_serializer.EndObjectArrayElement();
		}
	}

	_serializer.EndObjectArray();
}

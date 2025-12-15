#include "ECSWrapper.hpp"
#include "CheckpointEditor.hpp"

void cp::EntityAsset::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Entity Name", name);

	_serializer.BeginObjectArrayWriting("Components");

	for (auto component : components)
	{
		_serializer.BeginObjectArrayElementWriting();

		std::type_index typeIndex = std::type_index(typeid(*component));
		std::string componentTypeStr = cp::ComponentRegistry::GetInstance().GetTypeIndexMap().at(typeIndex);
		_serializer.WriteString("Type", componentTypeStr);
		cp::ISerializable* componentSerializer = cp::ComponentRegistry::GetInstance().CreateSerializer(typeIndex, component).release();

		_serializer.BeginObjectWriting("Data");
		componentSerializer->Serialize(_serializer);

		_serializer.EndObject();
		_serializer.EndObjectArrayElement();

		delete componentSerializer;
	}

	_serializer.EndObjectArray();
}

void cp::EntityAsset::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Entity Name", "Entity");

	if (size_t elements = _serializer.BeginObjectArrayReading("Components")) 
	{
		for (uint64_t index = 0; index < elements; index++) 
		{
			if (!_serializer.BeginObjectArrayElementReading(index)) continue;

			std::string componentTypeStr = _serializer.ReadString("Type", "No String");
			cp::IComponentBase* component = static_cast<cp::IComponentBase*>(
				cp::ComponentRegistry::GetInstance().CreateComponentInstance(componentTypeStr)
			);

			if (!component) {
				throw std::runtime_error("Couldn't deserialize component of type: " + componentTypeStr);
			}

			AddComponent(component);

			_serializer.BeginObjectReading("Data");

			cp::ISerializable* componentSerializer = cp::ComponentRegistry::GetInstance().CreateSerializer(
				std::type_index(typeid(*component)), component
			).release();

			componentSerializer->Deserialize(_serializer);

			_serializer.EndObject();
			_serializer.EndObjectArrayElement();

			delete componentSerializer;
		}
	}
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

void cp::SceneAsset::SaveScene()
{
	cp::JsonSerializer serializer;
	Serialize(serializer);

	if (path.empty()) {
		path = cp::CheckpointEditor::CurrentProject.GetResourcePath() + "/" + name + ".cpscene";
	}

	serializer.Write(path);
}
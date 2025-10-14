#include "pch.hpp"
#include "Scene.hpp"

#include "../ECS/Component/ComponentRegistry.hpp"

#include "../Resources/ResourceManager.hpp"
#include "../Resources/Mesh.hpp"
#include "../Resources/Texture.hpp"
#include "../Resources/Material.hpp"
#include "../Resources/MaterialInstance.hpp"

cp::Scene::Scene(cp::Renderer* _renderer) : renderer(_renderer)
{
	LOG_INFO("Scene created");

	cp::ResourceManager::Get()->RegisterResourceType<cp::Mesh>();
	cp::ResourceManager::Get()->GetResourceType<cp::Mesh>()->SetLoader(std::bind(&cp::Mesh::LoadMesh, std::placeholders::_1, std::placeholders::_2));
	cp::ResourceManager::Get()->RegisterResourceType<cp::Texture>();
	cp::ResourceManager::Get()->GetResourceType<cp::Texture>()->SetLoader(std::bind(&cp::Texture::LoadTexture, std::placeholders::_1, std::placeholders::_2));
	cp::ResourceManager::Get()->RegisterResourceType<cp::Material>();
	cp::ResourceManager::Get()->RegisterResourceType<cp::MaterialInstance>();

#ifdef IN_EDITOR
	sceneName = "Untitled scene";
#endif

	LOG_DEBUG(MF("ECS adress: {}", (void*)&ecs));
}

cp::Scene::~Scene()
{
	
}

void cp::Scene::Cleanup()
{
	ecs.Cleanup();
}

void cp::Scene::Update(float dt)
{
	ecs.Update(dt);
}

void cp::Scene::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Name", sceneName);

	_serializer.BeginObjectArrayWriting("Entities");

	for (const Entity& entity : ecs.GetEntities())
	{
		_serializer.BeginObjectArrayElementWriting();
		Entity::Serialize(entity, ecs.GetAllComponentsOf(entity), _serializer);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

void cp::Scene::Deserialize(ISerializer& _serializer)
{
	sceneName = _serializer.ReadString("Name", "Name error");

	size_t elements = -1;
	if ((elements = _serializer.BeginObjectArrayReading("Entities")) > 0)
	{
		for (uint64_t index = 0; index < elements; index++)
		{
			if (!_serializer.BeginObjectArrayElementReading(index)) continue; //Is continue the thing to do here ? Shouldn't I break instead ?

			Entity readEntity = ecs.CreateEntity();
			Entity::Deserialize(readEntity, ecs, _serializer);

			_serializer.EndObjectArrayElement();
		}

		_serializer.EndObjectArray();
	}
}
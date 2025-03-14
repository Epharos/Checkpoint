#include "pch.hpp"
#include "Scene.hpp"

#include "../ECS/Component/ComponentRegistry.hpp"

#include "../Resources/ResourceManager.hpp"
#include "../Resources/Mesh.hpp"
#include "../Resources/Texture.hpp"
#include "../Resources/Material.hpp"
#include "../Resources/MaterialInstance.hpp"

Core::Scene::Scene(Render::Renderer* _renderer) : renderer(_renderer)
{
	LOG_INFO("Scene created");

	Resource::ResourceManager::Get()->RegisterResourceType<Resource::Mesh>();
	Resource::ResourceManager::Get()->GetResourceType<Resource::Mesh>()->SetLoader(std::bind(&Resource::Mesh::LoadMesh, std::placeholders::_1, std::placeholders::_2));
	Resource::ResourceManager::Get()->RegisterResourceType<Resource::Texture>();
	Resource::ResourceManager::Get()->GetResourceType<Resource::Texture>()->SetLoader(std::bind(&Resource::Texture::LoadTexture, std::placeholders::_1, std::placeholders::_2));
	Resource::ResourceManager::Get()->RegisterResourceType<Resource::Material>();
	Resource::ResourceManager::Get()->RegisterResourceType<Resource::MaterialInstance>();

#ifdef IN_EDITOR
	sceneName = "Untitled scene";
#endif
}

Core::Scene::~Scene()
{
	
}

void Core::Scene::Cleanup()
{
	ecs.Cleanup();
}

void Core::Scene::Update(float dt)
{
	ecs.Update(dt);
}

void Core::Scene::Serialize(ISerializer& _serializer) const
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

void Core::Scene::Deserialize(ISerializer& _serializer)
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
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

void Core::Scene::Serialize(Serializer& _serializer) const
{
	_serializer.WriteString("name", sceneName);

	const void** entities = new const void* [ecs.GetEntities().size()];

	for (size_t i = 0; i < ecs.GetEntities().size(); i++)
	{
		entities[i] = &ecs.GetEntities()[i];
	}

	_serializer.WriteObjectArray("Entities", ecs.GetEntities().size(), entities, [&](const void* _entity, Serializer& _s)
		{
			const Entity* entity = static_cast<const Entity*>(_entity);
			Entity::Serialize(*entity, ecs.GetAllComponentsOf(*entity), _s);
		});
}

void Core::Scene::Deserialize(Serializer& _serializer)
{

}
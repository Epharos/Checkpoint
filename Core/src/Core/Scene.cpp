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

	_serializer.BeginObjectArray("Entities");

	for (const Entity& entity : ecs.GetEntities())
	{
		_serializer.BeginObjectArrayElement();
		Entity::Serialize(entity, ecs.GetAllComponentsOf(entity), _serializer);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

void Core::Scene::Deserialize(Serializer& _serializer)
{

}
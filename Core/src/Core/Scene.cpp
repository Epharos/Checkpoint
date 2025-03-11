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

	_serializer.WriteObjectArray("entities", ecs.GetEntities().size(), [&_serializer, this](Serializer& _s)
		{
			for (const auto& entity : ecs.GetEntities())
			{
				_s.WriteObject("entity", [&_serializer, this, entity, ecs](Serializer& _s)
					{
						_s.WriteInt("id", entity.id);

						_s.WriteObjectArray("components", ecs.GetAllComponentsOf(entity).size(), [&_serializer, this, entity](Serializer& _s)
							{
								for (const auto& [type, component] : ecs.GetAllComponentsOf(entity))
								{
									_s.WriteObject("component", [&_serializer, this, type, component](Serializer& _s)
										{
											_s.WriteString("type", ComponentRegistry::GetInstance().GetTypeIndexMap().at(type));
											ComponentRegistry::GetInstance().CreateSerializer(type)->Serialize(component, _s);
										});
								}
							});
					});
			}
		});
}

QJsonObject Core::Scene::Serialize()
{
	QJsonObject obj;

	obj["name"] = QString::fromStdString(sceneName);

	QJsonArray entitiesArray;

	for (const auto& entity : ecs.GetEntities())
	{
		QJsonObject entityObj;

		entityObj["id"] = static_cast<qint64>(entity.id);

		QJsonArray componentsArray;

		for (const auto& [type, component] : ecs.GetAllComponentsOf(entity))
		{
			QJsonObject componentObj;

			componentObj["type"] = QString::fromStdString(ComponentRegistry::GetInstance().GetTypeIndexMap().at(type));
			componentObj["data"] = ComponentRegistry::GetInstance().CreateSerializer(type)->Serialize(component);

			componentsArray.append(componentObj);
		}

		entityObj["components"] = componentsArray;

		entitiesArray.append(entityObj);
	}

	obj["entities"] = entitiesArray;

	return obj;
}

void Core::Scene::Deserialize(const QJsonObject& _data)
{
#ifdef IN_EDITOR
	sceneName = _data["name"].toString().toStdString();
#endif

	ecs.Cleanup();

	QJsonArray entitiesArray = _data["entities"].toArray();

	for (const auto& entity : entitiesArray)
	{
		Entity e = ecs.CreateEntity();

		QJsonObject entityObj = entity.toObject();

		e.id = entityObj["id"].toInt();

		QJsonArray componentsArray = entityObj["components"].toArray();

		for (const auto& component : componentsArray)
		{
			QJsonObject componentObj = component.toObject();

			std::string type = componentObj["type"].toString().toStdString();
			QJsonObject data = componentObj["data"].toObject();

			if (!ComponentRegistry::GetInstance().CreateComponent(ecs, e, type))
			{
				throw std::runtime_error("Couldn't create component of type: " + type);
			}

			ComponentRegistry::GetInstance().CreateSerializer(type)->Deserialize(data, ecs.GetComponent(e, type));
		}
	}
}
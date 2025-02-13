#include "pch.hpp"
#include "Scene.hpp"

Core::Scene::Scene(Render::Renderer* _renderer) : renderer(_renderer)
{
	LOG_INFO("Scene created");
}

Core::Scene::~Scene()
{
	
}

void Core::Scene::Cleanup()
{
	ecs.Cleanup();
	renderer->Cleanup();
}

void Core::Scene::Update(float dt)
{
	ecs.Update(dt);
}

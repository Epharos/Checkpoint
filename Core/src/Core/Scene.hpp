#pragma once

#include "../pch.hpp"
#include "../Render/Renderer/Renderer.hpp"

namespace Core
{
	class Scene
	{
	protected:
		ECS::EntityComponentSystem ecs;
		Render::Renderer* renderer;
	public:
		Scene(Render::Renderer* _renderer);
		~Scene();

		void Cleanup();

		void Update(float dt);

		ECS::EntityComponentSystem& GetECS() { return ecs; }
		Render::Renderer* GetRenderer() { return renderer; }
	};
}
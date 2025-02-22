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

#ifdef IN_EDITOR
		std::string sceneName;
#endif
	public:
		Scene(Render::Renderer* _renderer);
		~Scene();

		void Cleanup();

		void Update(float dt);

		inline constexpr ECS::EntityComponentSystem& GetECS() { return ecs; }
		inline constexpr Render::Renderer* GetRenderer() { return renderer; }

#ifdef IN_EDITOR
		inline void SetName(const std::string& name) { sceneName = name; }
		inline constexpr const std::string GetName() const { return sceneName; }
#endif
	};
}
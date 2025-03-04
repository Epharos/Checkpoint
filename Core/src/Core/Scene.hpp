#pragma once

#include "../pch.hpp"
#include "../Render/Renderer/Renderer.hpp"

#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>

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

		QJsonObject Serialize();
		void Deserialize(const QJsonObject& _data);

		inline constexpr ECS::EntityComponentSystem& GetECS() { return ecs; }
		inline constexpr Render::Renderer* GetRenderer() { return renderer; }

#ifdef IN_EDITOR
		inline void SetName(const std::string& name) { sceneName = name; }
		inline constexpr const std::string GetName() const { return sceneName; }
#endif
	};
}
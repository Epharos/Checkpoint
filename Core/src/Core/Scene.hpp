#pragma once

#include "../pch.hpp"
#include "../Render/Renderer/Renderer.hpp"
#include "../ECS/EntityComponentSystem.hpp"

#include "../Util/Serializers/Serializable.hpp"
#include "../Util/Serializers/Serializer.hpp"


namespace Core
{
	class Scene : public Serializable
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

		void Serialize(Serializer& _serializer) const override;
		void Deserialize(Serializer& _serializer) override;

		inline constexpr ECS::EntityComponentSystem& GetECS() { return ecs; }
		inline constexpr Render::Renderer* GetRenderer() { return renderer; }

#ifdef IN_EDITOR
		inline void SetName(const std::string& name) { sceneName = name; }
		inline constexpr const std::string GetName() const { return sceneName; }
#endif
	};
}
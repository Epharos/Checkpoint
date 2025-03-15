#pragma once

#include "../pch.hpp"
#include "../Render/Renderer/Renderer.hpp"
#include "../ECS/EntityComponentSystem.hpp"

#include "../Util/Serializers/Serializable.hpp"
#include "../Util/Serializers/ISerializer.hpp"

namespace cp
{
	class Scene : public ISerializable
	{
	protected:
		cp::EntityComponentSystem ecs;
		cp::Renderer* renderer;

#ifdef IN_EDITOR
		std::string sceneName;
#endif
	public:
		Scene(cp::Renderer* _renderer);
		~Scene();

		void Cleanup();

		void Update(float dt);

		void Serialize(cp::ISerializer& _serializer) const override;
		void Deserialize(cp::ISerializer& _serializer) override;

		inline constexpr cp::EntityComponentSystem& GetECS() { return ecs; }
		inline constexpr cp::Renderer* GetRenderer() { return renderer; }

#ifdef IN_EDITOR
		inline void SetName(const std::string& name) { sceneName = name; }
		inline constexpr const std::string GetName() const { return sceneName; }
#endif
	};
}
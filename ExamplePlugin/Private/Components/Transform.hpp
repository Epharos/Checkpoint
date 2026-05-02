#pragma once

#include <ECS/ECS.hpp>
#include <Common/Plugin/ComponentAuthoring.hpp>

namespace cp
{
	struct Transform
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		float pitch = 0.0f;
		float yaw = 0.0f;
		float roll = 0.0f;

		float scaleX = 1.0f;
		float scaleY = 1.0f;
		float scaleZ = 1.0f;
	};

	inline constexpr cp::ecs::TypeGuid TransformGuid = cp::ecs::MakeTypeGuid("ExamplePlugin.Transform");

	class TransformRegistrar final : public cp::ecs::IComponentRegistrar
	{
	public:
		[[nodiscard]] std::string_view Name() const override;
		[[nodiscard]] cp::ecs::TypeGuid ComponentGuid() const override;
		void Register(cp::ecs::World& _world) const override;
	};

	class TransformAuthoring final : public cp::IComponentAuthoring
	{
	public:
		[[nodiscard]] std::string_view Name() const override;
		[[nodiscard]] uint64_t ComponentGuid() const override;
		[[nodiscard]] bool HasComponent(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const override;
		[[nodiscard]] bool AddComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const override;
		[[nodiscard]] bool RemoveComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const override;
		[[nodiscard]] std::vector<cp::AuthoringSectionDescriptor> BuildSections(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const override;
		[[nodiscard]] bool ApplyValue(cp::ecs::World& _world, const cp::ecs::Entity& _entity, const std::string_view _fieldId, const cp::AuthoringValue& _value, std::string& _outError) const override;
	};
}
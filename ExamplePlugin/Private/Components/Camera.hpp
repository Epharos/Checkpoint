#pragma once

#include <ECS/ECS.hpp>
#include <Common/Plugin/ComponentAuthoring.hpp>

namespace cp
{
	struct Camera
	{
		float fovYDegrees = 60.0f;
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
		bool isPrimary = true;
		bool enabled = true;
	};

	inline constexpr cp::ecs::TypeGuid CameraGuid = cp::ecs::MakeTypeGuid("ExamplePlugin.Camera");

	class CameraRegistrar final : public cp::ecs::IComponentRegistrar
	{
	public:
		[[nodiscard]] std::string_view Name() const override;
		[[nodiscard]] cp::ecs::TypeGuid ComponentGuid() const override;
		void Register(cp::ecs::World& _world) const override;
	};

	class CameraAuthoring final : public cp::IComponentAuthoring
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

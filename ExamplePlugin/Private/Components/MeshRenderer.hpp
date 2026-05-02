#pragma once

#include <ECS/ECS.hpp>
#include <Resources/Mesh.hpp>
#include <Resources/AssetHandle.hpp>
#include <Common/Plugin/ComponentAuthoring.hpp>

namespace cp
{
	struct MeshRenderer
	{
		AssetHandle<Mesh> meshId {};
		bool visible = true;
	};

	inline constexpr cp::ecs::TypeGuid MeshRendererGuid = cp::ecs::MakeTypeGuid("ExamplePlugin.MeshRenderer");

	class MeshRendererRegistrar final : public cp::ecs::IComponentRegistrar
	{
	public:
		[[nodiscard]] std::string_view Name() const override;
		[[nodiscard]] cp::ecs::TypeGuid ComponentGuid() const override;
		void Register(cp::ecs::World& _world) const override;
	};

	class MeshRendererAuthoring final : public cp::IComponentAuthoring
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

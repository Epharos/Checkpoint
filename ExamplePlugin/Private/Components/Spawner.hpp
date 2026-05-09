#pragma once

#include <Common/Plugin/ComponentAuthoring.hpp>

#include <Resources/Mesh.hpp>
#include <Resources/AssetHandle.hpp>

#include <ECS/ECS.hpp>

namespace cp
{
    struct Spawner
    {
        double delay = 1.0;
        double currentDelay = 0.0;

        AssetHandle<Mesh> mesh;
    };

    inline constexpr cp::ecs::TypeGuid SpawnerGuid = cp::ecs::MakeTypeGuid("ExamplePlugin.Spawner");

    class SpawnerRegistrar final : public ecs::IComponentRegistrar
    {
    public:
        [[nodiscard]] std::string_view Name() const override;
        [[nodiscard]] ecs::TypeGuid ComponentGuid() const override;
        void Register(ecs::World& _world) const override;
    };

    class SpawnerAuthoring final : public cp::IComponentAuthoring
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
#pragma once

#include <iostream>
#include <ECS/ECS.hpp>

#include "../Components/Components.hpp"

namespace cp
{
    inline constexpr ecs::TypeGuid SpinSystemGuid = ecs::MakeTypeGuid("ExamplePlugin.SpinSystem");

    class SpinSystem final : public ecs::ISystem
    {
    public:
        [[nodiscard]] ecs::TypeGuid SystemGuid() const override
        {
            return SpinSystemGuid;
        }

        [[nodiscard]] std::string_view Name() const override
        {
            return "SpinSystem";
        }

        void Run(ecs::World& _world, ecs::CommandBuffer& _commandBuffer, float _deltaTime) override
        {
            _world.RunSystem(
                ecs::ReadAccess<MeshRenderer>{},
                ecs::WriteAccess<Transform>{},
                [_deltaTime](const ecs::Entity _entity, const MeshRenderer& _meshRenderer, Transform& _transform)
                {
                    _transform.yaw += _deltaTime * 90.f;
                }
            );
        }
    };
}
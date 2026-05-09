#pragma once

#include <ECS/ECS.hpp>

#include "../Components/Components.hpp"

namespace cp
{
    inline constexpr ecs::TypeGuid ShooterSystemGuid = ecs::MakeTypeGuid("ExamplePlugin.ShooterSystem");

    class ShooterSystem final : public ecs::ISystem
    {
    public:
        [[nodiscard]] ecs::TypeGuid SystemGuid() const override { return ShooterSystemGuid; }
        [[nodiscard]] std::string_view Name() const override { return "ShooterSystem"; }

        void Run(
            ecs::World& _world,
            ecs::CommandBuffer& _commandBuffer,
            float _deltaTime
        ) override
        {
            _world.RunSystem(
                ecs::ReadAccess<Transform>{},
                ecs::WriteAccess<Spawner>{},
                [_deltaTime, &_commandBuffer](const ecs::Entity _entity, const Transform& _transform, Spawner& _spawner) {
                    _spawner.currentDelay += _deltaTime;

                    if (_spawner.currentDelay >= _spawner.delay)
                    {
                        _spawner.currentDelay -= _spawner.delay;

                        const auto shotEntity = _commandBuffer.CreateEntity();
                        _commandBuffer.EmplaceComponent<Transform>(shotEntity, _transform);
                        _commandBuffer.EmplaceComponent<MeshRenderer>(shotEntity, MeshRenderer {
                            .meshId = _spawner.mesh,
                            .visible = true,
                        });
                        _commandBuffer.EmplaceComponent<RigidBody>(shotEntity, RigidBody {
                            .speedX = -1.f,
                            .speedY = 0.2f,
                            .speedZ = 0.f,
                        });
                    }
                }
            );
        }
    };
}
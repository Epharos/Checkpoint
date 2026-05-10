#pragma once

#include <ECS/ECS.hpp>

#include "../Components/Components.hpp"

namespace cp
{
    inline constexpr ecs::TypeGuid PhysicsSystemGuid = ecs::MakeTypeGuid("ExamplePlugin.PhysicsSystem");

    class PhysicsSystem final : public ecs::ISystem
    {
    public:
        [[nodiscard]] ecs::TypeGuid SystemGuid() const override { return PhysicsSystemGuid; }
        [[nodiscard]] std::string_view Name() const override { return "PhysicsSystem"; }

        [[nodiscard]] float GetGravity() const { return gravity; }
        void SetGravity(float _gravity) { gravity = _gravity; }

        void Serialize(cp::ISerializer& _serializer) const override
        {
            _serializer.WritePod(gravity);
        }

        void Deserialize(cp::IDeserializer& _deserializer) override
        {
            [[maybe_unused]] bool flag = _deserializer.ReadPod(gravity);
        }

        void Run(
            ecs::World& _world,
            ecs::CommandBuffer& _commandBuffer,
            float _deltaTime
        ) override
        {
            _world.RunSystem(
                ecs::ReadAccess<>{},
                ecs::WriteAccess<Transform, RigidBody>{},
                [_deltaTime, &_commandBuffer, this](const ecs::Entity _entity, Transform& _transform, RigidBody& _rigidbody) {
                    _rigidbody.speedY += gravity * _deltaTime;

                    _transform.x += _rigidbody.speedX * _deltaTime;
                    _transform.y += _rigidbody.speedY * _deltaTime;
                    _transform.z += _rigidbody.speedZ * _deltaTime;
                }
            );
        }

    protected:
        float gravity = -9.8f;
    };
}
#pragma once

#include <ECS/ECS.hpp>

#include "../Components/Components.hpp"

namespace cp
{
    inline constexpr ecs::TypeGuid SpinSystemGuid = ecs::MakeTypeGuid("ExamplePlugin.SpinSystem");

    class SpinSystem final : public ecs::ISystem
    {
    public:
        [[nodiscard]] ecs::TypeGuid SystemGuid() const override { return SpinSystemGuid; }
        [[nodiscard]] std::string_view Name() const override { return "SpinSystem"; }

        [[nodiscard]] float GetRotationSpeed() const { return rotationSpeed; }
        void SetRotationSpeed(float _speed) { rotationSpeed = _speed; }

        void Serialize(cp::ISerializer& _serializer) const override
        {
            _serializer.WritePod(rotationSpeed);
        }

        void Deserialize(cp::IDeserializer& _deserializer) override
        {
            [[maybe_unused]] bool flag = _deserializer.ReadPod(rotationSpeed);
        }

        void Run(ecs::World& _world, ecs::CommandBuffer& _commandBuffer, float _deltaTime) override
        {
            _world.RunSystem(
                ecs::ReadAccess<MeshRenderer>{},
                ecs::WriteAccess<Transform>{},
                [_deltaTime, speed = rotationSpeed](const ecs::Entity, const MeshRenderer&, Transform& _transform)
                {
                    _transform.yaw += _deltaTime * speed;
                }
            );
        }

    private:
        float rotationSpeed = 90.f;
    };
}

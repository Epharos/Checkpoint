#pragma once

#include "TypeGuid.hpp"

#include <string_view>

namespace cp::ecs
{
    class World;
    class CommandBuffer;

    /**
     * Runtime-instantiable ECS system interface.
     * Implementations are expected to be registered in a Registry<ISystem>.
     */
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        [[nodiscard]] virtual TypeGuid SystemGuid() const = 0;
        [[nodiscard]] virtual std::string_view Name() const = 0;
        virtual void Run(World& world, CommandBuffer& commandBuffer, float deltaTime) = 0;
    };
}

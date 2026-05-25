#pragma once

#include "TypeGuid.hpp"

#include <Common/Serialization/ISerializer.hpp>

#include <string_view>
#include <vector>

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

        [[nodiscard]] virtual std::vector<TypeGuid> RunAfter() const { return {}; }
        [[nodiscard]] virtual std::vector<TypeGuid> RunBefore() const { return {}; }

        /**
         * @brief Serialize the system's authored state to a binary stream.
         * Default no-op — override to persist system parameters.
         */
        virtual void Serialize(cp::ISerializer& _serializer) const {}

        /**
         * @brief Deserialize and restore the system's authored state from a binary stream.
         * Default no-op — override alongside Serialize to restore system parameters.
         */
        virtual void Deserialize(cp::IDeserializer& _deserializer) {}

        virtual void Run(World& world, CommandBuffer& commandBuffer, float deltaTime) = 0;
    };
}

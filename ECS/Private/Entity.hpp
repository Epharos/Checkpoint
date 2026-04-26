#pragma once

#include <cstdint>
#include <functional>
#include <limits>

namespace cp::ecs
{
    /**
     * Stable entity handle: index in world storage + generation counter.
     */
    struct Entity
    {
        static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

        uint32_t index = InvalidIndex;
        uint32_t generation = 0;

        [[nodiscard]] bool IsValid() const
        {
            return index != InvalidIndex;
        }

        [[nodiscard]] bool operator==(const Entity& _other) const = default;
    };

    constexpr auto InvalidEntity = Entity {};

}

template<>
struct std::hash<cp::ecs::Entity>
{
    [[nodiscard]] size_t operator()(const cp::ecs::Entity& _entity) const noexcept
    {
        return (static_cast<size_t>(_entity.index) << 32) ^ static_cast<size_t>(_entity.generation);
    }
};

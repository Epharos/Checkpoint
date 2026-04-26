#pragma once

#include "TypeGuid.hpp"

#include <string_view>

namespace cp::ecs
{
    class World;

    /**
     * Plugin-side component metadata registration contract.
     * Runtime creates registrars from a registry and applies them to a world.
     */
    class IComponentRegistrar
    {
    public:
        virtual ~IComponentRegistrar() = default;

        [[nodiscard]] virtual std::string_view Name() const = 0;
        [[nodiscard]] virtual TypeGuid ComponentGuid() const = 0;
        virtual void Register(World& world) const = 0;
    };
}

#pragma once

#include <Common/Plugin/SystemAuthoring.hpp>

#include <ECS/System.hpp>

#include "PhysicsSystem.hpp"

namespace cp
{
    class PhysicsSystemAuthoring final : public ISystemAuthoring
    {
    public:
        [[nodiscard]] std::string_view Name() const override { return "PhysicsSystem"; }
        [[nodiscard]] ecs::TypeGuid SystemGuid() const override { return PhysicsSystemGuid; }

        [[nodiscard]] std::vector<AuthoringSectionDescriptor> BuildSections(
            const ecs::ISystem& _system
        ) const override
        {
            const auto* phys = dynamic_cast<const PhysicsSystem*>(&_system);
            const double g = phys ? static_cast<double>(phys->GetGravity()) : -9.8;

            return {{
                .id = "physics",
                .title = "PhysicsSystem",
                .fields = {{
                    .id = "gravityCoeff",
                    .label = "Gravity (m.s^-2)",
                    .valueType = AuthoringValueType::Float,
                    .value = g,
                }}
            }};
        }

        [[nodiscard]] bool ApplyValue(
            ecs::ISystem& _system,
            std::string_view _fieldId,
            const AuthoringValue& _value,
            std::string& _outError
        ) const override
        {
            auto* phys = dynamic_cast<PhysicsSystem*>(&_system);
            if (!phys)
            {
                _outError = "System is not a PhysicsSystem";
                return false;
            }
            if (_fieldId == "gravityCoeff")
            {
                if (const double* v = std::get_if<double>(&_value))
                {
                    phys->SetGravity(static_cast<float>(*v));
                    return true;
                }
                _outError = "Expected a float value for gravityCoeff";
                return false;
            }
            _outError = "Unknown field: ";
            _outError += _fieldId;
            return false;
        }
    };
}

#pragma once

#include <Common/Plugin/SystemAuthoring.hpp>

#include <ECS/System.hpp>

#include "SpinSystem.hpp"

namespace cp
{
    class SpinSystemAuthoring final : public ISystemAuthoring
    {
    public:
        [[nodiscard]] std::string_view Name() const override { return "SpinSystem"; }
        [[nodiscard]] ecs::TypeGuid SystemGuid() const override { return SpinSystemGuid; }

        [[nodiscard]] std::vector<AuthoringSectionDescriptor> BuildSections(
            const ecs::ISystem& _system
        ) const override
        {
            const auto* spin = dynamic_cast<const SpinSystem*>(&_system);
            const double speed = spin ? static_cast<double>(spin->GetRotationSpeed()) : 90.0;
            return {{
                .id = "spin",
                .title = "SpinSystem",
                .fields = {{
                    .id = "rotationSpeed",
                    .label = "Rotation Speed (deg/s)",
                    .valueType = AuthoringValueType::Float,
                    .value = speed,
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
            auto* spin = dynamic_cast<SpinSystem*>(&_system);
            if (!spin)
            {
                _outError = "System is not a SpinSystem";
                return false;
            }
            if (_fieldId == "rotationSpeed")
            {
                if (const double* v = std::get_if<double>(&_value))
                {
                    spin->SetRotationSpeed(static_cast<float>(*v));
                    return true;
                }
                _outError = "Expected a float value for rotationSpeed";
                return false;
            }
            _outError = "Unknown field: ";
            _outError += _fieldId;
            return false;
        }
    };
}

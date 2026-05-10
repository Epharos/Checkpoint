#pragma once

#include <Common/Plugin/RenderPassAuthoring.hpp>

#include "NegativePFX.hpp"

namespace cp
{
    class NegativePostFXAuthoring final : public IRenderPassAuthoring
    {
    public:
        [[nodiscard]] std::string_view Name() const override { return "Negative Post-FX"; }
        [[nodiscard]] std::string_view PassRegistryName() const override { return NegativePostFXTypeName; }

        [[nodiscard]] std::vector<AuthoringSectionDescriptor> BuildSections(
            const IRenderPass& _pass) const override
        {
            const auto* pfx = dynamic_cast<const NegativePostFX*>(&_pass);
            const bool enabled = pfx ? pfx->GetEnabled() : true;

            return {{
                .id = "negativePFX",
                .title = "Negative Post-FX",
                .fields = {{
                    .id = "enabled",
                    .label = "Enabled",
                    .valueType = AuthoringValueType::Bool,
                    .value = enabled
                }}
            }};
        }

        [[nodiscard]] bool ApplyValue(
            IRenderPass& _pass,
            std::string_view _fieldId,
            const AuthoringValue& _value,
            std::string& _outError) const override
        {
            auto* pfx = dynamic_cast<NegativePostFX*>(&_pass);
            if (!pfx)
            {
                _outError = "Pass is not a NegativePostFX";
                return false;
            }

            if (_fieldId == "enabled")
            {
                if (const bool* v = std::get_if<bool>(&_value))
                {
                    pfx->SetEnabled(*v);
                    return true;
                }
                _outError = "Expected a bool value for 'enabled'";
                return false;
            }

            _outError = "Unknown field: ";
            _outError += _fieldId;
            return false;
        }
    };
}

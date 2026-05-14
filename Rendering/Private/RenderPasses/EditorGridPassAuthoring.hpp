#pragma once

#include <Common/Plugin/RenderPassAuthoring.hpp>

#include "EditorGridPass.hpp"

namespace cp
{
    class EditorGridPassAuthoring final : public IRenderPassAuthoring
    {
    public:
        [[nodiscard]] std::string_view Name() const override { return "Editor Grid"; }
        [[nodiscard]] std::string_view PassRegistryName() const override { return EditorGridPassTypeName; }

        [[nodiscard]] std::vector<AuthoringSectionDescriptor> BuildSections(
            const IRenderPass& _pass) const override
        {
            const auto* grid = dynamic_cast<const EditorGridPass*>(&_pass);

            const bool enabled = grid ? grid->GetEnabled() : true;
            const glm::vec3 color = grid ? grid->GetColor() : glm::vec3(0.5f);
            const double alpha = grid ? static_cast<double>(grid->GetAlpha()) : 0.8;
            const double cellSize = grid ? static_cast<double>(grid->GetCellSize()) : 1.0;
            const double thickness = grid ? static_cast<double>(grid->GetLineThickness()) : 0.02;
            const bool ignoreDepth = grid ? grid->GetIgnoreDepth() : false;

            return {{
                .id = "editorGrid",
                .title = "Editor Grid",
                .fields = {
                    {
                        .id = "enabled",
                        .label = "Enabled",
                        .valueType = AuthoringValueType::Bool,
                        .value = enabled
                    },
                    {
                        .id = "color",
                        .label = "Color (RGB)",
                        .valueType = AuthoringValueType::Vec3,
                        .value = AuthoringVec3 {
                            static_cast<double>(color.r),
                            static_cast<double>(color.g),
                            static_cast<double>(color.b)
                        }
                    },
                    {
                        .id = "alpha",
                        .label = "Opacity",
                        .valueType = AuthoringValueType::Float,
                        .value = alpha
                    },
                    {
                        .id = "cellSize",
                        .label = "Cell size (world units)",
                        .valueType = AuthoringValueType::Float,
                        .value = cellSize
                    },
                    {
                        .id = "lineThickness",
                        .label = "Line thickness [0 ; 0.5]",
                        .valueType = AuthoringValueType::Float,
                        .value = thickness
                    },
                    {
                        .id = "ignoreDepth",
                        .label = "Always on top",
                        .valueType = AuthoringValueType::Bool,
                        .value = ignoreDepth
                    },
                }
            }};
        }

        [[nodiscard]] bool ApplyValue(
            IRenderPass&     _pass,
            std::string_view _fieldId,
            const AuthoringValue& _value,
            std::string&     _outError) const override
        {
            auto* grid = dynamic_cast<EditorGridPass*>(&_pass);
            if (!grid)
            {
                _outError = "Pass is not an EditorGridPass";
                return false;
            }

            if (_fieldId == "enabled")
            {
                if (const bool* v = std::get_if<bool>(&_value))
                    { grid->SetEnabled(*v); return true; }
                _outError = "Expected bool for 'enabled'";
                return false;
            }

            if (_fieldId == "color")
            {
                if (const AuthoringVec3* v = std::get_if<AuthoringVec3>(&_value))
                {
                    grid->SetColor(glm::vec3(
                        static_cast<float>(v->x),
                        static_cast<float>(v->y),
                        static_cast<float>(v->z)
                    ));
                    return true;
                }
                _outError = "Expected Vec3 for 'color'";
                return false;
            }

            if (_fieldId == "alpha")
            {
                if (const double* v = std::get_if<double>(&_value))
                    { grid->SetAlpha(static_cast<float>(*v)); return true; }
                _outError = "Expected float for 'alpha'";
                return false;
            }

            if (_fieldId == "cellSize")
            {
                if (const double* v = std::get_if<double>(&_value))
                    { grid->SetCellSize(static_cast<float>(*v)); return true; }
                _outError = "Expected float for 'cellSize'";
                return false;
            }

            if (_fieldId == "lineThickness")
            {
                if (const double* v = std::get_if<double>(&_value))
                    { grid->SetLineThickness(static_cast<float>(*v)); return true; }
                _outError = "Expected float for 'lineThickness'";
                return false;
            }

            if (_fieldId == "ignoreDepth")
            {
                if (const bool* v = std::get_if<bool>(&_value))
                    { grid->SetIgnoreDepth(*v); return true; }
                _outError = "Expected bool for 'ignoreDepth'";
                return false;
            }

            _outError = "Unknown field: ";
            _outError += _fieldId;
            return false;
        }
    };
}

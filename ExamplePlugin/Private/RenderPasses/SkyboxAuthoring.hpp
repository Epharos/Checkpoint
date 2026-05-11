#pragma once

#include <Common/Plugin/RenderPassAuthoring.hpp>

#include "SkyboxPass.hpp"

namespace cp
{
    class SkyboxAuthoring final : public IRenderPassAuthoring
    {
    public:
        [[nodiscard]] std::string_view Name() const override { return "Skybox"; }
        [[nodiscard]] std::string_view PassRegistryName() const override { return SkyboxPassTypeName; }

        [[nodiscard]] std::vector<AuthoringSectionDescriptor> BuildSections(
            const IRenderPass& _pass
        ) const override
        {
            const auto* skybox = dynamic_cast<const SkyboxPass*>(&_pass);
            const std::string currentPath = skybox ? std::string(skybox->GetCubemapBasePath()) : std::string{};

            return {{
                .id = "skybox",
                .title = "Skybox",
                .fields = {{
                    .id = "cubemapPath",
                    .label = "Cubemap base path",
                    .valueType = AuthoringValueType::String,
                    .inputType = AuthoringInputType::FilePath,
                    .value = currentPath
                }}
            }};
        }

        [[nodiscard]] bool ApplyValue(
            IRenderPass& _pass,
            std::string_view _fieldId,
            const AuthoringValue& _value,
            std::string& _outError
        ) const override
        {
            auto* skybox = dynamic_cast<SkyboxPass*>(&_pass);
            if (!skybox)
            {
                _outError = "Pass is not a SkyboxPass";
                return false;
            }

            if (_fieldId == "cubemapPath")
            {
                if (const std::string* v = std::get_if<std::string>(&_value))
                {
                    skybox->SetCubemapPath(*v);
                    return true;
                }
                _outError = "Expected a string value for 'cubemapPath'";
                return false;
            }

            _outError = "Unknown field: ";
            _outError += _fieldId;
            return false;
        }
    };
}

#pragma once

#include <string>
#include <vector>

namespace cp
{
    class ISerializer;
    class IDeserializer;
    class Renderer;
}

namespace cp::rendering
{
    bool SerializeFrameGraphConfig(
        const std::vector<std::string>& _activePassNames,
        cp::ISerializer& _serializer
    );

    bool DeserializeFrameGraphConfig(
        std::vector<std::string>& _outActivePassNames,
        cp::IDeserializer& _deserializer
    );

    bool ApplyFrameGraphConfigToRenderer(
        cp::Renderer& _renderer,
        const std::vector<std::string>& _activePassNames
    );
}

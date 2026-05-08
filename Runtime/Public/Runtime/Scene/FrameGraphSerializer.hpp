#pragma once

#include <string>
#include <vector>

namespace cp
{
    class ISerializer;
    class IDeserializer;
    class ILogger;
}

namespace cp::runtime
{
    class FrameGraphSerializer
    {
    public:
        [[nodiscard]] static bool Serialize(
            const std::vector<std::string>& _activePassNames,
            cp::ISerializer& _serializer,
            cp::ILogger* _logger = nullptr
        );

        [[nodiscard]] static bool Deserialize(
            std::vector<std::string>& _outActivePassNames,
            cp::IDeserializer& _deserializer,
            cp::ILogger* _logger = nullptr
        );
    };
}

#include <Runtime/Scene/FrameGraphSerializer.hpp>

#include <Common/Serialization/ISerializer.hpp>

namespace cp::runtime
{
    bool FrameGraphSerializer::Serialize(
        const std::vector<std::string>& _activePassNames,
        cp::ISerializer& _serializer,
        cp::ILogger* _logger
    )
    {
        const auto count = static_cast<uint32_t>(_activePassNames.size());
        _serializer.WritePod(count);
        for (const auto& name : _activePassNames)
            _serializer.WriteString(name);

        return true;
    }

    bool FrameGraphSerializer::Deserialize(
        std::vector<std::string>& _outActivePassNames,
        cp::IDeserializer& _deserializer,
        cp::ILogger* _logger
    )
    {
        uint32_t count = 0;
        if (!_deserializer.ReadPod(count))
            return false;

        _outActivePassNames.resize(count);
        for (auto& name : _outActivePassNames)
        {
            if (!_deserializer.ReadString(name))
                return false;
        }

        return true;
    }
}

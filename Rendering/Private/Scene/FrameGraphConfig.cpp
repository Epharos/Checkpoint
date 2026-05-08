#include <Rendering/Scene/FrameGraphConfig.hpp>

#include <Common/Serialization/ISerializer.hpp>
#include <Rendering/Renderer.hpp>

namespace cp::rendering
{
    bool SerializeFrameGraphConfig(
        const std::vector<std::string>& _activePassNames,
        cp::ISerializer& _serializer
    )
    {
        const auto count = static_cast<uint32_t>(_activePassNames.size());
        _serializer.WritePod(count);

        for (const auto& name : _activePassNames)
            _serializer.WriteString(name);

        return true;
    }

    bool DeserializeFrameGraphConfig(
        std::vector<std::string>& _outActivePassNames,
        cp::IDeserializer& _deserializer
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

    bool ApplyFrameGraphConfigToRenderer(
        cp::Renderer& _renderer,
        const std::vector<std::string>& _activePassNames
    )
    {
        _renderer.ResetFrameGraph();

        for (const auto& passName : _activePassNames)
            _renderer.AddFrameGraphPass(passName);

        _renderer.RecompileFrameGraph();
        return true;
    }
}

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

    /**
     * @brief Reset the renderer's framegraph and rebuild it with the given active passes.
     *
     * Pass state blobs should be injected into the renderer BEFORE calling this function via
     * Renderer::SetPendingPassBlobs(), so that the newly-created pass instances have their
     * authored state restored during BuildFrameGraph().
     */
    bool ApplyFrameGraphConfigToRenderer(
        cp::Renderer& _renderer,
        const std::vector<std::string>& _activePassNames
    );
}

#pragma once

#include <cstdint>

namespace cp
{
    /**
     * @brief Describes in which context the renderer is being used.
     *
     * Stored in RendererInfo and forwarded to every render pass via
     * RenderPassExecutionContext so passes can adapt their behaviour
     * (e.g. use the editor camera, enable overlay rendering, etc.).
     */
    enum class RendererEnvironment : uint8_t
    {
        Editor,
        Runtime,
        DevelopmentBuild
    };
}

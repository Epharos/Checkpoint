#pragma once

#include <cstdint>

namespace cp
{
    enum class Format : uint32_t
    {
        Unknown,

        R8G8B8A8_UNORM,
        B8G8R8A8_UNORM,

        R16G16B16A16_FLOAT,

        R32_UINT,

        D24_UNORM_S8_UINT,
        D32_FLOAT,

        Count
    };

    enum class PipelineStage : uint32_t
    {
        Top = 1 << 0,
        DrawIndirect = 1 << 1,
        VertexInput = 1 << 2,
        VertexShader = 1 << 3,
        FragmentShader = 1 << 4,
        EarlyDepth = 1 << 5,
        LateDepth = 1 << 6,
        ColorAttachment = 1 << 7,
        ComputeShader = 1 << 8,
        Transfer = 1 << 9,
        Bottom = 1 << 10,
        AllGraphics = 1 << 11,
        AllCommands = 1 << 12
    };

    enum class Access : uint32_t
    {
        None = 1 << 0,

        IndirectCommandRead = 1 << 1,

        VertexBufferRead = 1 << 2,
        IndexBufferRead = 1 << 3,

        ShaderRead = 1 << 4,
        ShaderWrite = 1 << 5,

        ColorAttachmentRead = 1 << 6,
        ColorAttachmentWrite = 1 << 7,

        DepthStencilRead = 1 << 8,
        DepthStencilWrite = 1 << 9,

        TransferRead = 1 << 10,
        TransferWrite = 1 << 11,
    };
}
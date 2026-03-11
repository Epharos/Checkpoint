#pragma once

#include <cstdint>

namespace cp
{
    enum class TextureLayout : uint32_t;
    enum class Access : uint32_t;
    enum class PipelineStage : uint32_t;
    class ITexture;

    enum class BarrierType : uint32_t
    {
        Texture,
        Buffer
    };

    struct TextureBarrierInfo
    {
        ITexture& texture;

        TextureLayout srcLayout;
        TextureLayout dstLayout;

        PipelineStage srcStage;
        PipelineStage dstStage;

        Access srcAccess;
        Access dstAccess;

        uint32_t baseMip;
        uint32_t mipCount;

        uint32_t baseLayer;
        uint32_t layerCount;
    };

    struct BufferBarrierInfo
    {
        // TODO : Buffers
    };

    class IBarrier
    {
    public:
        explicit IBarrier(const TextureBarrierInfo& _barrierInfo) : type(BarrierType::Texture), textureBarrierInfo(_barrierInfo) {}
        explicit IBarrier(const BufferBarrierInfo& _barrierInfo) : type(BarrierType::Buffer), bufferBarrierInfo(_barrierInfo) {}

        BarrierType GetType() const { return type; }

        TextureBarrierInfo GetTextureBarrierInfo() const { return textureBarrierInfo; }
        BufferBarrierInfo GetBufferBarrierInfo() const { return bufferBarrierInfo; }

    protected:

        BarrierType type;

        union
        {
            TextureBarrierInfo textureBarrierInfo;
            BufferBarrierInfo bufferBarrierInfo;
        };


    };
}

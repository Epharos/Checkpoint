#include "FramegraphResource.hpp"

#include <Common/Core/Assert.hpp>

namespace cp
{
    FramegraphResource::FramegraphResource(std::string _name, const TextureInfo& _textureInfo, const bool _isTransient)
        : name(std::move(_name))
        , type(Type::Texture)
        , state(State::Virtual)
        , isTransient(_isTransient)
        , textureInfo(_textureInfo)
        , physicalTexture(nullptr)
    {
    }

    FramegraphResource::FramegraphResource(std::string _name, const BufferInfo& _bufferInfo, const bool _isTransient)
        : name(std::move(_name))
        , type(Type::Buffer)
        , state(State::Virtual)
        , isTransient(_isTransient)
        , bufferInfo(_bufferInfo)
        , physicalBuffer(nullptr)
    {
    }

    FramegraphResource::FramegraphResource(std::string _name, std::shared_ptr<ITexture> _texture)
        : name(std::move(_name))
        , type(Type::Texture)
        , state(State::Physical)
        , isTransient(false)
        , textureInfo{_texture->GetTextureInfo()}
        , physicalTexture(std::move(_texture))
    {
    }

    FramegraphResource::FramegraphResource(std::string _name, std::shared_ptr<IBuffer> _buffer)
        : name(std::move(_name))
        , type(Type::Buffer)
        , state(State::Physical)
        , isTransient(false)
        , bufferInfo{_buffer->GetBufferInfo()}
        , physicalBuffer(std::move(_buffer))
    {
    }

    FramegraphResource::~FramegraphResource()
    {
        DestroyResource();
        DestroyInfo();
    }

    FramegraphResource::FramegraphResource(FramegraphResource&& _other) noexcept
        : name(std::move(_other.name))
        , type(_other.type)
        , state(_other.state)
        , isTransient(_other.isTransient)
    {
        if (type == Type::Texture)
        {
            new (&textureInfo) TextureInfo(_other.textureInfo);

            if (state == State::Physical)
                new (&physicalTexture) std::shared_ptr<ITexture>(std::move(_other.physicalTexture));
        }
        else
        {
            new (&bufferInfo) BufferInfo(_other.bufferInfo);

            if (state == State::Physical)
                new (&physicalBuffer) std::shared_ptr<IBuffer>(std::move(_other.physicalBuffer));
        }

        _other.state = State::Virtual;
    }

    FramegraphResource& FramegraphResource::operator=(FramegraphResource&& _other) noexcept
    {
        if (this != &_other)
        {
            DestroyResource();
            DestroyInfo();

            name = std::move(_other.name);
            type = _other.type;
            state = _other.state;
            isTransient = _other.isTransient;

            if (type == Type::Texture)
            {
                new (&textureInfo) TextureInfo(_other.textureInfo);

                if (state == State::Physical)
                    new (&physicalTexture) std::shared_ptr<ITexture>(std::move(_other.physicalTexture));
            }
            else
            {
                new (&bufferInfo) BufferInfo(_other.bufferInfo);

                if (state == State::Physical)
                    new (&physicalBuffer) std::shared_ptr<IBuffer>(std::move(_other.physicalBuffer));
            }

            _other.state = State::Virtual;
        }

        return *this;
    }

    void FramegraphResource::AllocateTexture(std::shared_ptr<ITexture> _texture)
    {
        CP_EXPECT_MSG(type == Type::Texture, "Trying to allocate texture on a buffer resource");
        CP_EXPECT_MSG(state == State::Virtual, "Resource already allocated");
        CP_EXPECT_MSG(_texture != nullptr, "Cannot allocate null texture");

        new (&physicalTexture) std::shared_ptr<ITexture>(std::move(_texture));
        state = State::Physical;
    }

    void FramegraphResource::AllocateBuffer(std::shared_ptr<IBuffer> _buffer)
    {
        CP_EXPECT_MSG(type == Type::Buffer, "Trying to allocate buffer on a texture resource");
        CP_EXPECT_MSG(state == State::Virtual, "Resource already allocated");
        CP_EXPECT_MSG(_buffer != nullptr, "Cannot allocate null buffer");

        new (&physicalBuffer) std::shared_ptr<IBuffer>(std::move(_buffer));
        state = State::Physical;
    }

    void FramegraphResource::Reset()
    {
        if (isTransient && state == State::Physical)
        {
            DestroyResource();
            state = State::Virtual;
        }
    }

    const TextureInfo& FramegraphResource::GetTextureInfo() const
    {
        CP_EXPECT_MSG(type == Type::Texture, "Resource is not a texture");
        return textureInfo;
    }

    const BufferInfo& FramegraphResource::GetBufferInfo() const
    {
        CP_EXPECT_MSG(type == Type::Buffer, "Resource is not a buffer");
        return bufferInfo;
    }

    ITexture* FramegraphResource::GetTexture() const
    {
        CP_EXPECT_MSG(type == Type::Texture, "Resource is not a texture");
        CP_EXPECT_MSG(state == State::Physical, "Texture not allocated yet");
        
        ITexture* result = physicalTexture.get();
        CP_ENSURE_MSG(result != nullptr, "Texture pointer is null despite being in Physical state");
        
        return result;
    }

    IBuffer* FramegraphResource::GetBuffer() const
    {
        CP_EXPECT_MSG(type == Type::Buffer, "Resource is not a buffer");
        CP_EXPECT_MSG(state == State::Physical, "Buffer not allocated yet");
        
        IBuffer* result = physicalBuffer.get();
        CP_ENSURE_MSG(result != nullptr, "Buffer pointer is null despite being in Physical state");
        
        return result;
    }

    std::shared_ptr<ITexture> FramegraphResource::GetTextureShared() const
    {
        CP_EXPECT_MSG(type == Type::Texture, "Resource is not a texture");
        CP_EXPECT_MSG(state == State::Physical, "Texture not allocated yet");
        
        CP_ENSURE_MSG(physicalTexture != nullptr, "Texture shared_ptr is null despite being in Physical state");
        
        return physicalTexture;
    }

    std::shared_ptr<IBuffer> FramegraphResource::GetBufferShared() const
    {
        CP_EXPECT_MSG(type == Type::Buffer, "Resource is not a buffer");
        CP_EXPECT_MSG(state == State::Physical, "Buffer not allocated yet");
        
        CP_ENSURE_MSG(physicalBuffer != nullptr, "Buffer shared_ptr is null despite being in Physical state");
        
        return physicalBuffer;
    }

    void FramegraphResource::DestroyInfo()
    {
        if (type == Type::Texture)
        {
            textureInfo.~TextureInfo();
        }
        else
        {
            bufferInfo.~BufferInfo();
        }
    }

    void FramegraphResource::DestroyResource()
    {
        if (state == State::Physical)
        {
            if (type == Type::Texture)
            {
                physicalTexture.~shared_ptr<ITexture>();
            }
            else
            {
                physicalBuffer.~shared_ptr<IBuffer>();
            }
        }
    }
}

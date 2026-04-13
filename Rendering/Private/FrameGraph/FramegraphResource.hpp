#pragma once

#include <memory>
#include <string>

#include <RHI/Data.hpp>

namespace cp
{
    class ITexture;
    class IBuffer;

    enum class ResourceUsage : uint8_t
    {
        None = 0,
        WriteOnly = 1,
        ReadOnly = 2,
        ReadWrite = 3
    };

    constexpr ResourceUsage operator|(ResourceUsage _lhs, ResourceUsage _rhs)
    {
        return static_cast<ResourceUsage>(static_cast<uint32_t>(_lhs) | static_cast<uint32_t>(_rhs));
    }

    constexpr ResourceUsage& operator|=(ResourceUsage& _lhs, const ResourceUsage& _rhs)
    {
        _lhs = static_cast<ResourceUsage>(static_cast<uint32_t>(_lhs) | static_cast<uint32_t>(_rhs));
        return _lhs;
    }

    constexpr ResourceUsage operator&(ResourceUsage _lhs, ResourceUsage _rhs)
    {
        return static_cast<ResourceUsage>(static_cast<uint32_t>(_lhs) & static_cast<uint32_t>(_rhs));
    }

    /**
     * @brief Represents a resource (texture or buffer) in the framegraph.
     * 
     * A resource goes through several states:
     * 1. Virtual: Created during Setup(), only metadata exists
     * 2. Physical: Allocated during framegraph compilation, actual GPU resource exists
     *
     * Transient resources are managed by the framegraph.
     * Imported resources are external (e.g., swapchain images) and not destroyed by the framegraph.
     */
    class FramegraphResource
    {
    public:
        enum class Type
        {
            Texture,
            Buffer
        };

        enum class State
        {
            Virtual,   // Declared but not allocated
            Physical   // Allocated and ready to use
        };

        /**
         * @brief Create a virtual texture resource.
         * @param _name Resource name for debugging
         * @param _textureInfo Texture description
         * @param _isTransient If true, managed by framegraph; if false, external resource
         */
        FramegraphResource(std::string _name, const TextureInfo& _textureInfo, bool _isTransient = true);

        /**
         * @brief Create a virtual buffer resource.
         * @param _name Resource name for debugging
         * @param _bufferInfo Buffer description
         * @param _isTransient If true, managed by framegraph; if false, external resource
         */
        FramegraphResource(std::string _name, const BufferInfo& _bufferInfo, bool _isTransient = true);

        /**
         * @brief Create an imported texture resource (already allocated).
         * @param _name Resource name for debugging
         * @param _texture Existing texture
         */
        FramegraphResource(std::string _name, std::shared_ptr<ITexture> _texture);

        /**
         * @brief Create an imported buffer resource (already allocated).
         * @param _name Resource name for debugging
         * @param _buffer Existing buffer
         */
        FramegraphResource(std::string _name, std::shared_ptr<IBuffer> _buffer);

        ~FramegraphResource();

        FramegraphResource(const FramegraphResource&) = delete;
        FramegraphResource& operator=(const FramegraphResource&) = delete;
        FramegraphResource(FramegraphResource&&) noexcept;
        FramegraphResource& operator=(FramegraphResource&&) noexcept;

        /**
         * @brief Allocate the physical texture resource.
         * @param _texture The allocated texture
         */
        void AllocateTexture(std::shared_ptr<ITexture> _texture);

        /**
         * @brief Allocate the physical buffer resource.
         * @param _buffer The allocated buffer
         */
        void AllocateBuffer(std::shared_ptr<IBuffer> _buffer);

        /**
         * @brief Reset the resource (free if transient).
         * Called during FrameGraph reset phase.
         */
        void Reset();

        [[nodiscard]] std::string_view GetName() const { return name; }
        [[nodiscard]] Type GetType() const { return type; }
        [[nodiscard]] State GetState() const { return state; }
        [[nodiscard]] bool IsTransient() const { return isTransient; }
        [[nodiscard]] bool IsVirtual() const { return state == State::Virtual; }
        [[nodiscard]] bool IsPhysical() const { return state == State::Physical; }
        [[nodiscard]] bool IsTexture() const { return type == Type::Texture; }
        [[nodiscard]] bool IsBuffer() const { return type == Type::Buffer; }

        /**
         * @brief Get the texture info (valid for texture resources).
         */
        [[nodiscard]] const TextureInfo& GetTextureInfo() const;

        /**
         * @brief Get the buffer info (valid for buffer resources).
         */
        [[nodiscard]] const BufferInfo& GetBufferInfo() const;

        /**
         * @brief Get the physical texture (must be allocated first).
         * @return Pointer to the texture, or nullptr if not allocated
         */
        [[nodiscard]] ITexture* GetTexture() const;

        /**
         * @brief Get the physical buffer (must be allocated first).
         * @return Pointer to the buffer, or nullptr if not allocated
         */
        [[nodiscard]] IBuffer* GetBuffer() const;

        /**
         * @brief Get the shared pointer to the physical texture.
         */
        [[nodiscard]] std::shared_ptr<ITexture> GetTextureShared() const;

        /**
         * @brief Get the shared pointer to the physical buffer.
         */
        [[nodiscard]] std::shared_ptr<IBuffer> GetBufferShared() const;

    private:
        void DestroyInfo();
        void DestroyResource();

        std::string name;
        Type type;
        State state;
        bool isTransient;

        // Resource metadata (always present)
        union
        {
            TextureInfo textureInfo;
            BufferInfo bufferInfo;
        };

        // Physical resource (present after allocation)
        union
        {
            std::shared_ptr<ITexture> physicalTexture;
            std::shared_ptr<IBuffer> physicalBuffer;
        };
    };

    /**
     * @brief Handle to a framegraph resource for use in render passes.
     * Prevents direct modification of the resource by passes.
     */
    class FramegraphResourceHandle
    {
    public:
        explicit FramegraphResourceHandle(FramegraphResource* _resource)
            : resource(_resource)
        {}

        [[nodiscard]] ITexture* GetTexture() const { return resource->GetTexture(); }
        [[nodiscard]] IBuffer* GetBuffer() const { return resource->GetBuffer(); }
        [[nodiscard]] std::string_view GetName() const { return resource->GetName(); }
        [[nodiscard]] bool IsTexture() const { return resource->IsTexture(); }
        [[nodiscard]] bool IsBuffer() const { return resource->IsBuffer(); }

    private:
        FramegraphResource* resource;
    };
}

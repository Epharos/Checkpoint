#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>
#include <typeindex>
#include <type_traits>
#include <utility>

#include <Common/Core/Assert.hpp>

#include "FramegraphResource.hpp"

namespace cp
{
    class IRenderPass;

    /**
     * @brief Builder for declaring resources in a render pass.
     * 
     * Used during the Setup() phase of render passes to:
     * - Create new transient resources
     * - Import external resources
     * - Declare read/write dependencies
     * 
     * Example usage in a render pass:
     * @code
     * void Setup(FrameGraphBuilder& builder) override
     * {
     *     // Create a new color target
     *     data.colorOutput = builder.CreateTexture("ColorTarget", colorInfo);
     *     builder.UseTexture(data.colorOutput, {
     *         .layout = TextureLayout::ColorAttachment,
     *         .stage = PipelineStage::ColorAttachment,
     *         .access = Access::ColorAttachmentWrite
     *     }, ResourceAccess::WriteOnly);
     *     
     *     // Read from a previous resource
     *     builder.UseTexture(data.sceneColor, {
     *         .layout = TextureLayout::ShaderReadOnly,
     *         .stage = PipelineStage::FragmentShader,
     *         .access = Access::ShaderRead
     *     }, ResourceAccess::ReadOnly);
     * }
     * @endcode
     */
    class FrameGraphBuilder
    {
    public:
        FrameGraphBuilder();
        ~FrameGraphBuilder() = default;

        struct TextureSynchronization
        {
            TextureLayout layout = TextureLayout::Undefined;
            PipelineStage stage = PipelineStage::AllCommands;
            Access access = Access::None;
            uint32_t baseMip = 0;
            uint32_t mipCount = 1;
            uint32_t baseLayer = 0;
            uint32_t layerCount = 1;
        };

        struct BufferSynchronization
        {
            PipelineStage stage = PipelineStage::AllCommands;
            Access access = Access::None;
            uint64_t offsetBytes = 0;
            uint64_t sizeBytes = std::numeric_limits<uint64_t>::max();
        };

        /**
         * @brief Create a new transient texture managed by the framegraph.
         * @param _name Resource name for debugging
         * @param _info Texture description
         * @return Handle to the created resource
         */
        FramegraphResourceHandle* CreateTexture(const std::string& _name, const TextureInfo& _info);

        /**
         * @brief Create a new transient buffer managed by the framegraph.
         * @param _name Resource name for debugging
         * @param _info Buffer description
         * @return Handle to the created resource
         */
        FramegraphResourceHandle* CreateBuffer(const std::string& _name, const BufferInfo& _info);

        /**
         * @brief Import an external texture (e.g., swapchain image).
         * @param _name Resource name for debugging
         * @param _texture Existing texture
         * @return Handle to the imported resource
         */
        FramegraphResourceHandle* ImportTexture(const std::string& _name, std::shared_ptr<ITexture> _texture);

        /**
         * @brief Import an external buffer.
         * @param _name Resource name for debugging
         * @param _buffer Existing buffer
         * @return Handle to the imported resource
         */
        FramegraphResourceHandle* ImportBuffer(const std::string& _name, std::shared_ptr<IBuffer> _buffer);

        /**
         * @brief Share an existing CPU-side resource by name.
         * @tparam T Resource type
         * @param _name Resource name for retrieval
         * @param _resource Shared resource instance
         */
        template<typename T>
        void ShareCpuResource(const std::string& _name, std::shared_ptr<T> _resource)
        {
            static_assert(!std::is_void_v<T>, "CPU resource type cannot be void");
            CP_EXPECT_MSG(!_name.empty(), "CPU resource name cannot be empty");
            CP_EXPECT_MSG(_resource != nullptr, "Cannot share null CPU resource");

            const auto existing = cpuResourceTypes.find(_name);
            if (existing != cpuResourceTypes.end())
            {
                CP_ASSERT_MSG(existing->second == std::type_index(typeid(T)), "CPU resource already exists with a different type");
            }

            cpuResources.insert_or_assign(_name, std::move(_resource));
            cpuResourceTypes.insert_or_assign(_name, std::type_index(typeid(T)));
        }

        /**
         * @brief Construct and share a CPU-side resource by name.
         * @tparam T Resource type
         * @tparam Args Constructor args
         * @param _name Resource name for retrieval
         * @param _args Constructor args forwarded to T
         * @return Shared pointer to the stored resource
         */
        template<typename T, typename... Args>
        std::shared_ptr<T> EmplaceCpuResource(const std::string& _name, Args&&... _args)
        {
            std::shared_ptr<T> resource = std::make_shared<T>(std::forward<Args>(_args)...);
            ShareCpuResource(_name, resource);
            return resource;
        }

        /**
         * @brief Retrieve a shared CPU-side resource by name.
         * @tparam T Expected resource type
         * @param _name Resource name
         * @return Shared pointer to resource, nullptr if not found
         */
        template<typename T>
        [[nodiscard]] std::shared_ptr<T> GetCpuResource(const std::string& _name) const
        {
            static_assert(!std::is_void_v<T>, "CPU resource type cannot be void");
            CP_EXPECT_MSG(!_name.empty(), "CPU resource name cannot be empty");

            const auto it = cpuResources.find(_name);
            if (it == cpuResources.end())
            {
                return nullptr;
            }

            const auto typeIt = cpuResourceTypes.find(_name);
            CP_ASSERT_MSG(typeIt != cpuResourceTypes.end(), "CPU resource type metadata is missing");
            CP_ASSERT_MSG(typeIt->second == std::type_index(typeid(T)), "Requested CPU resource type does not match stored type");

            return std::static_pointer_cast<T>(it->second);
        }

        /**
         * @brief Check if a CPU-side resource exists.
         */
        [[nodiscard]] bool HasCpuResource(const std::string& _name) const;

        /**
         * @brief Remove a shared CPU-side resource.
         */
        void RemoveCpuResource(const std::string& _name);

        /**
         * @brief Declare the final synchronization state expected after the last pass touching a texture.
         * @param _handle Handle to the texture resource
         * @param _sync Final synchronization state for external usage after framegraph execution
         */
        void SetTextureFinalState(const FramegraphResourceHandle* _handle, const TextureSynchronization& _sync);

        /**
         * @brief Declare the final synchronization state expected after the last pass touching a buffer.
         * @param _handle Handle to the buffer resource
         * @param _sync Final synchronization state for external usage after framegraph execution
         */
        void SetBufferFinalState(const FramegraphResourceHandle* _handle, const BufferSynchronization& _sync);

        /**
         * @brief Use a texture by name in this pass with explicit access mode.
         * @param _name Resource name
         * @param _sync Synchronization state describing how this pass uses this texture
         * @param _access Access mode: read, write, or read-write
         * @return Handle to the resource, or nullptr if not found
         */
        FramegraphResourceHandle* UseTexture(
            const std::string& _name,
            const TextureSynchronization& _sync,
            ResourceUsage _access = ResourceUsage::ReadOnly
        );

        /**
         * @brief Use a texture handle in this pass with explicit access mode.
         * @param _handle Handle to the texture resource
         * @param _sync Synchronization state describing how this pass uses this texture
         * @param _access Access mode: read, write, or read-write
         * @return The same handle
         */
        FramegraphResourceHandle* UseTexture(
            FramegraphResourceHandle* _handle,
            const TextureSynchronization& _sync,
            ResourceUsage _access = ResourceUsage::ReadOnly
        );

        /**
         * @brief Use a buffer by name in this pass with explicit access mode.
         * @param _name Resource name
         * @param _sync Synchronization state describing how this pass uses this buffer
         * @param _access Access mode: read, write, or read-write
         * @return Handle to the resource, or nullptr if not found
         */
        FramegraphResourceHandle* UseBuffer(
            const std::string& _name,
            const BufferSynchronization& _sync,
            ResourceUsage _access = ResourceUsage::ReadOnly
        );

        /**
         * @brief Use a buffer handle in this pass with explicit access mode.
         * @param _handle Handle to the buffer resource
         * @param _sync Synchronization state describing how this pass uses this buffer
         * @param _access Access mode: read, write, or read-write
         * @return The same handle
         */
        FramegraphResourceHandle* UseBuffer(
            FramegraphResourceHandle* _handle,
            const BufferSynchronization& _sync,
            ResourceUsage _access = ResourceUsage::ReadOnly
        );

        /**
         * @brief Get a resource handle by name without declaring usage.
         * @param _name Resource name
         * @return Handle to the resource, or nullptr if not found
         * 
         * Useful when you need the handle but will declare usage later,
         * or when looking up resources from other passes.
         */
        FramegraphResourceHandle* GetResourceHandle(const std::string& _name);

        /**
         * @brief Mark the pass currently declaring resources.
         * Must be called before read/write declarations for that pass.
         */
        void BeginPassSetup(IRenderPass* _pass);

        /**
         * @brief Clear the current pass setup context.
         */
        void EndPassSetup();

        /**
         * @brief Get all resources created/imported by this builder.
         */
        [[nodiscard]] std::vector<std::unique_ptr<FramegraphResource>>& GetResources() { return resources; }

        /**
         * @brief Get resource by name.
         */
        [[nodiscard]] FramegraphResource* GetResourceByName(const std::string& _name);

        struct TextureResourceAccess
        {
            IRenderPass* pass = nullptr;
            FramegraphResource* resource = nullptr;
            TextureSynchronization sync {};
            bool isWrite = false;
        };

        struct BufferResourceAccess
        {
            IRenderPass* pass = nullptr;
            FramegraphResource* resource = nullptr;
            BufferSynchronization sync {};
            bool isWrite = false;
        };

        [[nodiscard]] const std::vector<TextureResourceAccess>& GetTextureAccesses() const { return textureAccesses; }
        [[nodiscard]] const std::vector<BufferResourceAccess>& GetBufferAccesses() const { return bufferAccesses; }
        [[nodiscard]] const std::unordered_map<FramegraphResource*, TextureSynchronization>& GetTextureFinalStates() const { return textureFinalStates; }
        [[nodiscard]] const std::unordered_map<FramegraphResource*, BufferSynchronization>& GetBufferFinalStates() const { return bufferFinalStates; }

        /**
         * @brief Clear all resources (used during reset).
         */
        void Clear();

    private:
        void ReadTexture(const FramegraphResourceHandle* _handle, const TextureSynchronization& _sync);
        void WriteTexture(const FramegraphResourceHandle* _handle, const TextureSynchronization& _sync);
        void ReadBuffer(const FramegraphResourceHandle* _handle, const BufferSynchronization& _sync);
        void WriteBuffer(const FramegraphResourceHandle* _handle, const BufferSynchronization& _sync);

        std::vector<std::unique_ptr<FramegraphResource>> resources;
        std::unordered_map<std::string, FramegraphResource*> resourceMap;
        std::vector<std::unique_ptr<FramegraphResourceHandle>> handles;
        std::unordered_map<std::string, FramegraphResourceHandle*> handleMap;

        std::vector<TextureResourceAccess> textureAccesses;
        std::vector<BufferResourceAccess> bufferAccesses;
        std::unordered_map<FramegraphResource*, TextureSynchronization> textureFinalStates;
        std::unordered_map<FramegraphResource*, BufferSynchronization> bufferFinalStates;
        std::unordered_map<std::string, std::shared_ptr<void>> cpuResources;
        std::unordered_map<std::string, std::type_index> cpuResourceTypes;
        IRenderPass* currentSetupPass = nullptr;
    };
}

#include "FrameGraphBuilder.hpp"

#include <Common/Core/Assert.hpp>

namespace cp
{
    FrameGraphBuilder::FrameGraphBuilder() = default;

    FramegraphResourceHandle* FrameGraphBuilder::CreateTexture(const std::string& _name, const TextureInfo& _info)
    {
        CP_EXPECT_MSG(!_name.empty(), "Resource name cannot be empty");
        CP_EXPECT_MSG(!resourceMap.contains(_name), "Resource with this name already exists");

        // Create the resource
        auto resource = std::make_unique<FramegraphResource>(_name, _info, true);
        auto* resourcePtr = resource.get();
        CP_ASSERT_MSG(resourcePtr != nullptr, "Failed to create FramegraphResource");

        // Create a handle
        auto handle = std::make_unique<FramegraphResourceHandle>(resourcePtr);
        auto* handlePtr = handle.get();
        CP_ASSERT_MSG(handlePtr != nullptr, "Failed to create FramegraphResourceHandle");

        // Store
        resourceMap[_name] = resourcePtr;
        resources.push_back(std::move(resource));
        handleMap[_name] = handlePtr;
        handles.push_back(std::move(handle));

        CP_ENSURE_MSG(handlePtr != nullptr, "Returned handle is null");
        return handlePtr;
    }

    FramegraphResourceHandle* FrameGraphBuilder::CreateBuffer(const std::string& _name, const BufferInfo& _info)
    {
        CP_EXPECT_MSG(!_name.empty(), "Resource name cannot be empty");
        CP_EXPECT_MSG(!resourceMap.contains(_name), "Resource with this name already exists");

        // Create the resource
        auto resource = std::make_unique<FramegraphResource>(_name, _info, true);
        auto* resourcePtr = resource.get();
        CP_ASSERT_MSG(resourcePtr != nullptr, "Failed to create FramegraphResource");

        // Create a handle
        auto handle = std::make_unique<FramegraphResourceHandle>(resourcePtr);
        auto* handlePtr = handle.get();
        CP_ASSERT_MSG(handlePtr != nullptr, "Failed to create FramegraphResourceHandle");

        // Store
        resourceMap[_name] = resourcePtr;
        resources.push_back(std::move(resource));
        handleMap[_name] = handlePtr;
        handles.push_back(std::move(handle));

        CP_ENSURE_MSG(handlePtr != nullptr, "Returned handle is null");
        return handlePtr;
    }

    FramegraphResourceHandle* FrameGraphBuilder::ImportTexture(const std::string& _name, std::shared_ptr<ITexture> _texture)
    {
        CP_EXPECT_MSG(_texture != nullptr, "Cannot import null texture");
        CP_EXPECT_MSG(!resourceMap.contains(_name), "Resource with this name already exists");

        // Create the imported resource
        auto resource = std::make_unique<FramegraphResource>(_name, std::move(_texture));
        auto* resourcePtr = resource.get();
        CP_ASSERT_MSG(resourcePtr != nullptr, "Failed to create FramegraphResource");

        // Create a handle
        auto handle = std::make_unique<FramegraphResourceHandle>(resourcePtr);
        auto* handlePtr = handle.get();
        CP_ASSERT_MSG(handlePtr != nullptr, "Failed to create FramegraphResourceHandle");

        // Store
        resourceMap[_name] = resourcePtr;
        resources.push_back(std::move(resource));
        handleMap[_name] = handlePtr;
        handles.push_back(std::move(handle));

        CP_ENSURE_MSG(handlePtr != nullptr, "Returned handle is null");
        return handlePtr;
    }

    FramegraphResourceHandle* FrameGraphBuilder::ImportBuffer(const std::string& _name, std::shared_ptr<IBuffer> _buffer)
    {
        CP_EXPECT_MSG(_buffer != nullptr, "Cannot import null buffer");
        CP_EXPECT_MSG(!resourceMap.contains(_name), "Resource with this name already exists");

        // Create the imported resource
        auto resource = std::make_unique<FramegraphResource>(_name, std::move(_buffer));
        auto* resourcePtr = resource.get();
        CP_ASSERT_MSG(resourcePtr != nullptr, "Failed to create FramegraphResource");

        // Create a handle
        auto handle = std::make_unique<FramegraphResourceHandle>(resourcePtr);
        auto* handlePtr = handle.get();
        CP_ASSERT_MSG(handlePtr != nullptr, "Failed to create FramegraphResourceHandle");

        // Store
        resourceMap[_name] = resourcePtr;
        resources.push_back(std::move(resource));
        handleMap[_name] = handlePtr;
        handles.push_back(std::move(handle));

        CP_ENSURE_MSG(handlePtr != nullptr, "Returned handle is null");
        return handlePtr;
    }

    void FrameGraphBuilder::ReadTexture(const FramegraphResourceHandle* _handle, const TextureSynchronization& _sync)
    {
        CP_EXPECT_MSG(_handle != nullptr, "Cannot read from null handle");
        CP_EXPECT_MSG(_handle->IsTexture(), "Handle is not a texture");
        CP_EXPECT_MSG(currentSetupPass != nullptr, "ReadTexture must be called during pass Setup()");

        FramegraphResource* resource = GetResourceByName(std::string(_handle->GetName()));
        CP_ASSERT_MSG(resource != nullptr, "Texture resource not found for read access");

        textureAccesses.push_back({ currentSetupPass, resource, _sync, false });
    }

    void FrameGraphBuilder::WriteTexture(const FramegraphResourceHandle* _handle, const TextureSynchronization& _sync)
    {
        CP_EXPECT_MSG(_handle != nullptr, "Cannot write to null handle");
        CP_EXPECT_MSG(_handle->IsTexture(), "Handle is not a texture");
        CP_EXPECT_MSG(currentSetupPass != nullptr, "WriteTexture must be called during pass Setup()");

        FramegraphResource* resource = GetResourceByName(std::string(_handle->GetName()));
        CP_ASSERT_MSG(resource != nullptr, "Texture resource not found for write access");

        textureAccesses.push_back({ currentSetupPass, resource, _sync, true });
    }

    void FrameGraphBuilder::ReadBuffer(const FramegraphResourceHandle* _handle, const BufferSynchronization& _sync)
    {
        CP_EXPECT_MSG(_handle != nullptr, "Cannot read from null handle");
        CP_EXPECT_MSG(_handle->IsBuffer(), "Handle is not a buffer");
        CP_EXPECT_MSG(currentSetupPass != nullptr, "ReadBuffer must be called during pass Setup()");

        FramegraphResource* resource = GetResourceByName(std::string(_handle->GetName()));
        CP_ASSERT_MSG(resource != nullptr, "Buffer resource not found for read access");

        bufferAccesses.push_back({ currentSetupPass, resource, _sync, false });
    }

    void FrameGraphBuilder::WriteBuffer(const FramegraphResourceHandle* _handle, const BufferSynchronization& _sync)
    {
        CP_EXPECT_MSG(_handle != nullptr, "Cannot write to null handle");
        CP_EXPECT_MSG(_handle->IsBuffer(), "Handle is not a buffer");
        CP_EXPECT_MSG(currentSetupPass != nullptr, "WriteBuffer must be called during pass Setup()");

        FramegraphResource* resource = GetResourceByName(std::string(_handle->GetName()));
        CP_ASSERT_MSG(resource != nullptr, "Buffer resource not found for write access");

        bufferAccesses.push_back({ currentSetupPass, resource, _sync, true });
    }

    void FrameGraphBuilder::SetTextureFinalState(
        const FramegraphResourceHandle* _handle,
        const TextureSynchronization& _sync
    )
    {
        CP_EXPECT_MSG(_handle != nullptr, "Cannot set texture final state on null handle");
        CP_EXPECT_MSG(_handle->IsTexture(), "Handle is not a texture");
        CP_EXPECT_MSG(currentSetupPass != nullptr, "SetTextureFinalState must be called during pass Setup()");

        FramegraphResource* resource = GetResourceByName(std::string(_handle->GetName()));
        CP_ASSERT_MSG(resource != nullptr, "Texture resource not found for final state");

        textureFinalStates[resource] = _sync;
    }

    void FrameGraphBuilder::SetBufferFinalState(
        const FramegraphResourceHandle* _handle,
        const BufferSynchronization& _sync
    )
    {
        CP_EXPECT_MSG(_handle != nullptr, "Cannot set buffer final state on null handle");
        CP_EXPECT_MSG(_handle->IsBuffer(), "Handle is not a buffer");
        CP_EXPECT_MSG(currentSetupPass != nullptr, "SetBufferFinalState must be called during pass Setup()");

        FramegraphResource* resource = GetResourceByName(std::string(_handle->GetName()));
        CP_ASSERT_MSG(resource != nullptr, "Buffer resource not found for final state");

        bufferFinalStates[resource] = _sync;
    }

    FramegraphResourceHandle* FrameGraphBuilder::UseTexture(
        const std::string& _name,
        const TextureSynchronization& _sync,
        const ResourceUsage _access
    )
    {
        auto* handle = GetResourceHandle(_name);
        CP_ASSERT_MSG(handle != nullptr, "Texture resource not found");
        CP_ASSERT_MSG(handle->IsTexture(), "Requested resource is not a texture");
        return UseTexture(handle, _sync, _access);
    }

    FramegraphResourceHandle* FrameGraphBuilder::UseTexture(
        FramegraphResourceHandle* _handle,
        const TextureSynchronization& _sync,
        const ResourceUsage _access
    )
    {
        CP_EXPECT_MSG(_handle != nullptr, "Cannot use null texture handle");
        CP_EXPECT_MSG(_handle->IsTexture(), "Handle is not a texture");
        CP_EXPECT_MSG(_access != ResourceUsage::None, "Texture usage access cannot be None");

        if ((_access & ResourceUsage::ReadOnly) == ResourceUsage::ReadOnly)
        {
            ReadTexture(_handle, _sync);
        }

        if ((_access & ResourceUsage::WriteOnly) == ResourceUsage::WriteOnly)
        {
            WriteTexture(_handle, _sync);
        }

        return _handle;
    }

    FramegraphResourceHandle* FrameGraphBuilder::UseBuffer(
        const std::string& _name,
        const BufferSynchronization& _sync,
        const ResourceUsage _access
    )
    {
        auto* handle = GetResourceHandle(_name);
        CP_ASSERT_MSG(handle != nullptr, "Buffer resource not found");
        CP_ASSERT_MSG(handle->IsBuffer(), "Requested resource is not a buffer");
        return UseBuffer(handle, _sync, _access);
    }

    FramegraphResourceHandle* FrameGraphBuilder::UseBuffer(
        FramegraphResourceHandle* _handle,
        const BufferSynchronization& _sync,
        const ResourceUsage _access
    )
    {
        CP_EXPECT_MSG(_handle != nullptr, "Cannot use null buffer handle");
        CP_EXPECT_MSG(_handle->IsBuffer(), "Handle is not a buffer");
        CP_EXPECT_MSG(_access != ResourceUsage::None, "Buffer usage access cannot be None");

        if ((_access & ResourceUsage::ReadOnly) == ResourceUsage::ReadOnly)
        {
            ReadBuffer(_handle, _sync);
        }

        if ((_access & ResourceUsage::WriteOnly) == ResourceUsage::WriteOnly)
        {
            WriteBuffer(_handle, _sync);
        }

        return _handle;
    }

    FramegraphResourceHandle* FrameGraphBuilder::GetResourceHandle(const std::string& _name)
    {
        const auto it = handleMap.find(_name);
        return (it != handleMap.end()) ? it->second : nullptr;
    }

    void FrameGraphBuilder::BeginPassSetup(IRenderPass* _pass)
    {
        CP_EXPECT_MSG(_pass != nullptr, "Cannot begin setup with null pass");
        CP_EXPECT_MSG(currentSetupPass == nullptr, "Previous pass setup context was not ended");
        currentSetupPass = _pass;
    }

    void FrameGraphBuilder::EndPassSetup()
    {
        currentSetupPass = nullptr;
    }

    FramegraphResource* FrameGraphBuilder::GetResourceByName(const std::string& _name)
    {
        const auto it = resourceMap.find(_name);
        return (it != resourceMap.end()) ? it->second : nullptr;
    }

    void FrameGraphBuilder::Clear()
    {
        resources.clear();
        resourceMap.clear();
        handles.clear();
        handleMap.clear();
        textureAccesses.clear();
        bufferAccesses.clear();
        textureFinalStates.clear();
        bufferFinalStates.clear();
        currentSetupPass = nullptr;
    }
}

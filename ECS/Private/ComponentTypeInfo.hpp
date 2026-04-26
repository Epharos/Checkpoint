#pragma once

#include <Common/Serialization/ISerializer.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <typeindex>

namespace cp::ecs
{
    using ComponentTypeId = uint32_t;
    using ComponentTypeGuid = uint64_t;

    /**
     * Runtime metadata used to manage component storage in archetypes.
     */
    struct ComponentTypeInfo
    {
        using DefaultConstructFn = void (*)(void*);
        using CopyConstructFn = void (*)(void*, const void*);
        using MoveConstructFn = void (*)(void*, void*);
        using DestroyFn = void (*)(void*);

        std::type_index type = typeid(void);
        ComponentTypeGuid typeGuid = 0;
        uint32_t schemaVersion = 1;
        size_t size = 0;
        size_t alignment = 1;

        DefaultConstructFn defaultConstruct = nullptr;
        CopyConstructFn copyConstruct = nullptr;
        MoveConstructFn moveConstruct = nullptr;
        DestroyFn destroy = nullptr;

        std::string stableName;
        std::function<void(const void*, ISerializer&)> serializeBinary;
        std::function<bool(void*, IDeserializer&)> deserializeBinary;
    };
}

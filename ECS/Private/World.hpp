#pragma once

#include "Access.hpp"
#include "Archetype.hpp"
#include "ComponentTypeInfo.hpp"
#include "Entity.hpp"
#include "TypeGuid.hpp"

#include <Common/Core/Assert.hpp>
#include <Common/Serialization/ISerializer.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <concepts>
#include <functional>
#include <limits>
#include <memory>
#include <new>
#include <span>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cp::ecs
{
    template<typename Component>
    concept MutableComponent = !std::is_const_v<Component> && !std::is_reference_v<Component>;

    template<typename Component>
    concept StorableComponent = MutableComponent<Component>
        && std::is_copy_constructible_v<Component>
        && std::is_move_constructible_v<Component>;

    /**
     * Central ECS world.
     * Owns entities, component type metadata, and all archetypes.
     */
    class World
    {
    public:
        World();
        ~World();

        World(const World&) = delete;
        World& operator=(const World&) = delete;
        World(World&&) = delete;
        World& operator=(World&&) = delete;

        /**
         * @brief Creates a new alive entity in the empty archetype.
         */
        [[nodiscard]] Entity CreateEntity();

        /**
         * @brief Destroys an entity and invalidates its previous handle generation.
         */
        [[nodiscard]] bool DestroyEntity(Entity _entity);

        /**
         * @brief Returns true if the handle points to a currently alive entity.
         */
        [[nodiscard]] bool IsAlive(Entity _entity) const;

        void ReserveEntities(size_t count);
        void Clear();
        void ClearComponentTypes();

        template<MutableComponent Component, typename... Args>
        Component& AddComponent(Entity _entity, Args&&... _args);

        template<MutableComponent Component>
        [[nodiscard]] bool RemoveComponent(Entity _entity);

        template<MutableComponent Component>
        [[nodiscard]] bool HasComponent(Entity _entity) const;

        template<MutableComponent Component>
        [[nodiscard]] Component* TryGetComponent(Entity _entity);

        template<MutableComponent Component>
        [[nodiscard]] const Component* TryGetComponent(Entity _entity) const;

        template<MutableComponent Component>
        Component& GetComponent(Entity _entity);

        template<MutableComponent Component>
        const Component& GetComponent(Entity _entity) const;

        /**
         * Executes a system over all entities matching the required read/write components.
         * Callback can be either:
         *  - fn(Entity, const TRead&..., TWrite&...)
         *  - fn(const TRead&..., TWrite&...)
         */
        template<typename... TRead, typename... TWrite, typename TFunc>
        requires (
            std::is_invocable_v<TFunc, Entity, const TRead&..., TWrite&...>
            || std::is_invocable_v<TFunc, const TRead&..., TWrite&...>
        )
        void RunSystem(ReadAccess<TRead...>, WriteAccess<TWrite...>, TFunc&& _func);

        /**
         * Registers binary serialization callbacks for a component.
         */
        template<MutableComponent TComponent>
        void RegisterComponentSerialization(
            ComponentTypeGuid _componentGuid,
            uint32_t _schemaVersion,
            std::string _stableName,
            std::function<void(const TComponent&, ISerializer&)> _serialize,
            std::function<bool(TComponent&, IDeserializer&)> _deserialize
        );

        void SetRequiredPlugins(std::vector<std::string> _pluginNames);
        [[nodiscard]] const std::vector<std::string>& GetRequiredPlugins() const;

        void SetStartupSystems(std::vector<TypeGuid> _systemGuids);
        [[nodiscard]] const std::vector<TypeGuid>& GetStartupSystems() const;
        [[nodiscard]] std::vector<Entity> GetAliveEntities() const;

        [[nodiscard]] bool SerializeBinary(ISerializer& _serializer) const;
        [[nodiscard]] bool DeserializeBinary(IDeserializer& _deserializer);

    private:
        struct EntityRecord
        {
            Archetype* archetype = nullptr;
            uint32_t generation = 1;
            uint32_t chunkIndex = 0;
            uint32_t rowIndex = 0;
            bool alive = false;
        };

        template<StorableComponent TComponent>
        [[nodiscard]] ComponentTypeId EnsureComponentType();

        [[nodiscard]] ComponentTypeId RegisterComponentType(const ComponentTypeInfo& _info);
        [[nodiscard]] EntityRecord* TryGetRecord(Entity _entity);
        [[nodiscard]] const EntityRecord* TryGetRecord(Entity _entity) const;

        [[nodiscard]] Archetype& GetOrCreateArchetype(const std::vector<ComponentTypeId>& _signature);
        [[nodiscard]] void* GetComponentPtr(const EntityRecord& _record, ComponentTypeId _componentId);
        [[nodiscard]] const void* GetComponentPtr(const EntityRecord& _record, ComponentTypeId _componentId) const;

        void RemoveEntityFromArchetype(const EntityRecord& _record);
        void MoveEntityToArchetype(
            Entity _entity,
            EntityRecord& _record,
            const std::vector<ComponentTypeId>& _targetSignature,
            const ComponentTypeInfo* _addedType,
            const void* _addedValue,
            ComponentTypeId _removedComponentId
        );

        [[nodiscard]] bool AddComponentFromUntyped(Entity _entity, ComponentTypeId _componentId, const void* _valuePtr);
        [[nodiscard]] bool DeserializeComponentPayload(Entity _entity, ComponentTypeId _componentId, const std::vector<std::byte>& _payload);
        [[nodiscard]] size_t ResolveChunkCapacity(const std::vector<ComponentTypeId>& _signature) const;

    private:
        static constexpr uint32_t SerializationMagic = 0x43504543; // CPEC
        static constexpr uint32_t SerializationVersion = 2;
        static constexpr ComponentTypeId InvalidComponentType = std::numeric_limits<ComponentTypeId>::max();

        std::vector<EntityRecord> entityRecords;
        std::vector<uint32_t> freeEntityIndices;

        std::vector<ComponentTypeInfo> componentTypes;
        std::unordered_map<std::type_index, ComponentTypeId> componentTypeByTypeIndex;
        std::unordered_map<ComponentTypeGuid, ComponentTypeId> componentTypeByGuid;

        std::vector<std::unique_ptr<Archetype>> archetypes;
        std::unordered_map<std::vector<ComponentTypeId>, Archetype*, SignatureHasher> archetypeBySignature;

        std::vector<std::string> requiredPlugins;
        std::vector<TypeGuid> startupSystems;
    };

    template<MutableComponent Component, typename... Args>
    Component& World::AddComponent(const Entity _entity, Args&&... _args)
    {
        EntityRecord* record = TryGetRecord(_entity);
        CP_ASSERT_MSG(record != nullptr, "Cannot add a component to an invalid entity.");

        const ComponentTypeId componentId = EnsureComponentType<Component>();
        if (void* componentPtr = GetComponentPtr(*record, componentId))
        {
            *static_cast<Component*>(componentPtr) = Component(std::forward<Args>(_args)...);
            return *static_cast<Component*>(componentPtr);
        }

        Component newValue(std::forward<Args>(_args)...);
        const bool added = AddComponentFromUntyped(_entity, componentId, &newValue);
        CP_ASSERT_MSG(added, "Failed to add component.");

        Component* result = TryGetComponent<Component>(_entity);
        CP_ASSERT_MSG(result != nullptr, "Component was not available after insertion.");
        return *result;
    }

    template<MutableComponent Component>
    bool World::RemoveComponent(const Entity _entity)
    {
        EntityRecord* record = TryGetRecord(_entity);
        if (!record)
        {
            return false;
        }

        const ComponentTypeId componentId = EnsureComponentType<Component>();
        if (!GetComponentPtr(*record, componentId))
        {
            return false;
        }

        std::vector<ComponentTypeId> targetSignature = record->archetype->Signature();
        const auto removeIt = std::lower_bound(targetSignature.begin(), targetSignature.end(), componentId);
        if (removeIt == targetSignature.end() || *removeIt != componentId)
        {
            return false;
        }

        targetSignature.erase(removeIt);
        MoveEntityToArchetype(_entity, *record, targetSignature, nullptr, nullptr, componentId);
        return true;
    }

    template<MutableComponent Component>
    bool World::HasComponent(const Entity _entity) const
    {
        const EntityRecord* record = TryGetRecord(_entity);
        if (!record)
        {
            return false;
        }

        const ComponentTypeId componentId = const_cast<World*>(this)->EnsureComponentType<Component>();
        return GetComponentPtr(*record, componentId) != nullptr;
    }

    template<MutableComponent Component>
    Component* World::TryGetComponent(const Entity _entity)
    {
        EntityRecord* record = TryGetRecord(_entity);
        if (record == nullptr)
        {
            return nullptr;
        }

        const ComponentTypeId componentId = EnsureComponentType<Component>();
        return static_cast<Component*>(GetComponentPtr(*record, componentId));
    }

    template<MutableComponent Component>
    const Component* World::TryGetComponent(const Entity _entity) const
    {
        return const_cast<World*>(this)->TryGetComponent<Component>(_entity);
    }

    template<MutableComponent Component>
    Component& World::GetComponent(const Entity _entity)
    {
        Component* component = TryGetComponent<Component>(_entity);
        CP_ASSERT_MSG(component != nullptr, "Requested component is not present on the entity.");
        return *component;
    }

    template<MutableComponent Component>
    const Component& World::GetComponent(const Entity _entity) const
    {
        return const_cast<World*>(this)->GetComponent<Component>(_entity);
    }

    template<typename... Read, typename... Write, typename Func>
    requires (
        std::is_invocable_v<Func, Entity, const Read&..., Write&...>
        || std::is_invocable_v<Func, const Read&..., Write&...>
    )
    void World::RunSystem(ReadAccess<Read...>, WriteAccess<Write...>, Func&& _func)
    {
        std::vector<ComponentTypeId> requiredComponentIds;
        requiredComponentIds.reserve(sizeof...(Read) + sizeof...(Write));

        (requiredComponentIds.push_back(EnsureComponentType<Read>()), ...);
        (requiredComponentIds.push_back(EnsureComponentType<Write>()), ...);

        std::sort(requiredComponentIds.begin(), requiredComponentIds.end());
        requiredComponentIds.erase(std::unique(requiredComponentIds.begin(), requiredComponentIds.end()), requiredComponentIds.end());

        for (const std::unique_ptr<Archetype>& archetypePtr : archetypes)
        {
            Archetype& archetype = *archetypePtr;
            if (!archetype.Contains(requiredComponentIds))
            {
                continue;
            }

            for (uint32_t chunkIndex = 0; chunkIndex < archetype.ChunkCount(); ++chunkIndex)
            {
                const auto rowCount = static_cast<uint32_t>(archetype.ChunkEntityCount(chunkIndex));
                for (uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex)
                {
                    const Entity entity = archetype.EntityAt(chunkIndex, rowIndex);
                    auto writePtr = [this, &archetype, chunkIndex, rowIndex]<typename T>() -> T*
                    {
                        const ComponentTypeId componentId = EnsureComponentType<T>();
                        return static_cast<T*>(archetype.ComponentPtr(componentId, chunkIndex, rowIndex));
                    };
                    auto readPtr = [this, &archetype, chunkIndex, rowIndex]<typename T>() -> const T*
                    {
                        const ComponentTypeId componentId = EnsureComponentType<T>();
                        return static_cast<const T*>(archetype.ComponentPtr(componentId, chunkIndex, rowIndex));
                    };

                    if constexpr (std::is_invocable_v<Func, Entity, const Read&..., Write&...>)
                    {
                        std::invoke(
                            _func,
                            entity,
                            *readPtr.template operator()<Read>()...,
                            *writePtr.template operator()<Write>()...
                        );
                    }
                    else
                    {
                        std::invoke(
                            _func,
                            *readPtr.template operator()<Read>()...,
                            *writePtr.template operator()<Write>()...
                        );
                    }
                }
            }
        }
    }

    template<MutableComponent Component>
    void World::RegisterComponentSerialization(
        const ComponentTypeGuid _componentGuid,
        const uint32_t _schemaVersion,
        std::string _stableName,
        std::function<void(const Component&, ISerializer&)> _serialize,
        std::function<bool(Component&, IDeserializer&)> _deserialize
    )
    {
        CP_ASSERT_MSG(_componentGuid != 0, "Component guid must be non-zero.");
        const ComponentTypeId componentId = EnsureComponentType<Component>();
        ComponentTypeInfo& info = componentTypes[componentId];

        if (_stableName.empty())
        {
            _stableName = typeid(Component).name();
        }

        info.typeGuid = _componentGuid;
        info.schemaVersion = _schemaVersion;
        info.stableName = std::move(_stableName);
        info.serializeBinary = [serializer = std::move(_serialize)](const void* _ptr, ISerializer& _writer)
        {
            serializer(*static_cast<const Component*>(_ptr), _writer);
        };
        info.deserializeBinary = [deserializer = std::move(_deserialize)](void* _ptr, IDeserializer& _reader)
        {
            return deserializer(*static_cast<Component*>(_ptr), _reader);
        };

        componentTypeByGuid[info.typeGuid] = componentId;
    }

    template<StorableComponent Component>
    ComponentTypeId World::EnsureComponentType()
    {
        const std::type_index typeIndex = typeid(Component);
        const auto it = componentTypeByTypeIndex.find(typeIndex);
        if (it != componentTypeByTypeIndex.end())
        {
            return it->second;
        }

        ComponentTypeInfo info{};
        info.type = typeIndex;
        info.typeGuid = 0;
        info.schemaVersion = 1;
        info.size = sizeof(Component);
        info.alignment = alignof(Component);
        if constexpr (std::is_default_constructible_v<Component>)
        {
            info.defaultConstruct = [](void* _dst) { new (_dst) Component(); };
        }
        else
        {
            info.defaultConstruct = nullptr;
        }
        info.copyConstruct = [](void* _dst, const void* _src) { new (_dst) Component(*static_cast<const Component*>(_src)); };
        info.moveConstruct = [](void* _dst, void* _src) { new (_dst) Component(std::move(*static_cast<Component*>(_src))); };
        info.destroy = [](void* _ptr) { static_cast<Component*>(_ptr)->~Component(); };
        info.stableName = typeIndex.name();

        const ComponentTypeId componentId = RegisterComponentType(info);
        componentTypeByTypeIndex[typeIndex] = componentId;
        return componentId;
    }
}

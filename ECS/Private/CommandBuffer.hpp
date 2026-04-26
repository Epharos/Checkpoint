#pragma once

#include "Entity.hpp"
#include "World.hpp"

#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cp::ecs
{
    /**
     * Defers structural ECS operations and replays them once systems finished.
     */
    class CommandBuffer
    {
    public:
        struct DeferredEntity
        {
            uint32_t id = 0;
        };

        [[nodiscard]] DeferredEntity CreateEntity()
        {
            const DeferredEntity deferred{nextDeferredId++};
            commands.push_back(std::make_unique<CreateEntityCommand>(deferred.id));
            return deferred;
        }

        void Destroy(const Entity _entity)
        {
            commands.push_back(std::make_unique<DestroyEntityCommand>(EntityRef::FromEntity(_entity)));
        }

        void Destroy(const DeferredEntity _entity)
        {
            commands.push_back(std::make_unique<DestroyEntityCommand>(EntityRef::FromDeferred(_entity.id)));
        }

        template<typename Component, typename... Args>
        void EmplaceComponent(const Entity _entity, Args&&... _args)
        {
            commands.push_back(std::make_unique<EmplaceComponentCommand<Component>>(
                EntityRef::FromEntity(_entity),
                Component(std::forward<Args>(_args)...)
            ));
        }

        template<typename Component, typename... Args>
        void EmplaceComponent(const DeferredEntity _entity, Args&&... _args)
        {
            commands.push_back(std::make_unique<EmplaceComponentCommand<Component>>(
                EntityRef::FromDeferred(_entity.id),
                Component(std::forward<Args>(_args)...)
            ));
        }

        template<typename Component>
        void RemoveComponent(const Entity _entity)
        {
            commands.push_back(std::make_unique<RemoveComponentCommand<Component>>(EntityRef::FromEntity(_entity)));
        }

        template<typename Component>
        void RemoveComponent(const DeferredEntity _entity)
        {
            commands.push_back(std::make_unique<RemoveComponentCommand<Component>>(EntityRef::FromDeferred(_entity.id)));
        }

        void Playback(World& _world)
        {
            PlaybackContext context{};

            for (const std::unique_ptr<ICommand>& command : commands)
            {
                command->Execute(_world, context);
            }

            Clear();
        }

        void Clear()
        {
            commands.clear();
            nextDeferredId = 1;
        }

    private:
        struct EntityRef
        {
            [[nodiscard]] static EntityRef FromEntity(const Entity _entity)
            {
                EntityRef ref{};
                ref.entity = _entity;
                ref.isDeferred = false;
                return ref;
            }

            [[nodiscard]] static EntityRef FromDeferred(const uint32_t _deferredId)
            {
                EntityRef ref{};
                ref.deferredId = _deferredId;
                ref.isDeferred = true;
                return ref;
            }

            bool isDeferred = false;
            Entity entity{};
            uint32_t deferredId = 0;
        };

        struct PlaybackContext
        {
            std::unordered_map<uint32_t, Entity> deferredToEntity;
        };

        struct ICommand
        {
            virtual ~ICommand() = default;
            virtual void Execute(World& _world, PlaybackContext& _context) const = 0;
        };

        [[nodiscard]] static std::optional<Entity> Resolve(const EntityRef& _ref, PlaybackContext& _context)
        {
            if (!_ref.isDeferred)
            {
                return _ref.entity;
            }

            const auto it = _context.deferredToEntity.find(_ref.deferredId);
            if (it == _context.deferredToEntity.end())
            {
                return std::nullopt;
            }

            return it->second;
        }

        struct CreateEntityCommand final : ICommand
        {
            explicit CreateEntityCommand(const uint32_t _deferredId)
                : deferredId(_deferredId)
            {
            }

            void Execute(World& _world, PlaybackContext& _context) const override
            {
                _context.deferredToEntity[deferredId] = _world.CreateEntity();
            }

            uint32_t deferredId = 0;
        };

        struct DestroyEntityCommand final : ICommand
        {
            explicit DestroyEntityCommand(const EntityRef _entityRef)
                : entityRef(_entityRef)
            {
            }

            void Execute(World& _world, PlaybackContext& _context) const override
            {
                const std::optional<Entity> resolvedEntity = Resolve(entityRef, _context);
                if (!resolvedEntity.has_value())
                {
                    return;
                }

                [[maybe_unused]] bool flag = _world.DestroyEntity(*resolvedEntity);
            }

            EntityRef entityRef{};
        };

        template<typename Component>
        struct EmplaceComponentCommand final : ICommand
        {
            EmplaceComponentCommand(const EntityRef _entityRef, Component&& _componentValue)
                : entityRef(_entityRef)
                , componentValue(std::move(_componentValue))
            {
            }

            void Execute(World& _world, PlaybackContext& _context) const override
            {
                const std::optional<Entity> resolvedEntity = Resolve(entityRef, _context);
                if (!resolvedEntity.has_value())
                {
                    return;
                }

                _world.AddComponent<Component>(*resolvedEntity, componentValue);
            }

            EntityRef entityRef{};
            Component componentValue;
        };

        template<typename Component>
        struct RemoveComponentCommand final : ICommand
        {
            explicit RemoveComponentCommand(const EntityRef _entityRef)
                : entityRef(_entityRef)
            {
            }

            void Execute(World& _world, PlaybackContext& _context) const override
            {
                const std::optional<Entity> resolvedEntity = Resolve(entityRef, _context);
                if (!resolvedEntity.has_value())
                {
                    return;
                }

                [[maybe_unused]] bool flag = _world.RemoveComponent<Component>(*resolvedEntity);
            }

            EntityRef entityRef{};
        };

    private:
        std::vector<std::unique_ptr<ICommand>> commands;
        uint32_t nextDeferredId = 1;
    };
}

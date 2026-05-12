#pragma once

namespace cp::ecs
{
    class World;
    struct Entity;
}

namespace cp
{
    /**
     * @brief Read-only access to the editor's current scene state.
     */
    class IEditorStateProvider
    {
    public:
        virtual ~IEditorStateProvider() = default;

        /**
         * @brief Returns the active ECS world, or nullptr if no scene is loaded.
         */
        virtual ecs::World* GetWorld() = 0;

        /**
         * @brief Returns a pointer to the currently selected entity,
         *        or nullptr if nothing is selected.
         */
        virtual const ecs::Entity* GetSelectedEntity() = 0;
    };
}

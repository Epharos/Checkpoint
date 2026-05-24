#pragma once

#include <string_view>

#include <Common/Data/Extent.hpp>

#include <ECS/Entity.hpp>

#include "Camera.hpp"
#include "DebugDraw.hpp"

namespace cp::ecs
{
    class World;
}

namespace cp
{
    enum class GizmoSpace : uint8_t
    {
        World,
        Local
    };

    /**
     * @brief Context passed to gizmo contributions during the render phase.
     * Filled by the editor each frame before BeginFrame().
     */
    struct GizmoRenderContext
    {
        DebugDrawBuffer& debugDrawBuffer;
        const ResolvedCameraData& camera;
        Extent2D<uint32_t> viewport;
        std::string_view activeTool;
        GizmoSpace space = GizmoSpace::World;
    };

    /**
     * @brief Context passed to gizmo contributions during mouse interaction.
     */
    struct GizmoInteractContext
    {
        const ResolvedCameraData& camera;
        Extent2D<uint32_t> viewport;
        std::string_view activeTool;
        GizmoSpace space = GizmoSpace::World;
    };

    /**
     * @brief Plugin-side interface for rendering and interacting with editor gizmo handles.
     *
     * Implementations are registered under GizmoContributionRegistryName.
     * Each frame the editor calls Render() for every contribution whose IsApplicable()
     * returns true for the selected entity.
     * Mouse press, move, release events are forwarded to contributions in registration order
     * the first contribution that returns true from OnMousePress() consumes the drag.
     */
    class IGizmoContribution
    {
    public:
        virtual ~IGizmoContribution() = default;

        /**
         * @brief Returns true when this contribution's gizmo handles should be active
         * for the given entity.
         */
        virtual bool IsApplicable(const ecs::World& _world, ecs::Entity _entity) const = 0;

        /**
         * @brief Fill the debug draw buffer with gizmo geometry for this frame.
         * Called once per frame for every applicable entity.
         */
        virtual void Render(
            const GizmoRenderContext& _ctx,
            const ecs::World& _world,
            ecs::Entity _entity
        ) = 0;

        /**
         * @brief Called when the left mouse button is pressed over the viewport.
         * @return true if this contribution consumed the event (i.e. started a drag).
         */
        virtual bool OnMousePress(
            int _x, int _y,
            const GizmoInteractContext& _ctx,
            ecs::World& _world,
            ecs::Entity _entity
        )
        {
            return false;
        }

        /**
         * @brief Called each frame the mouse moves while any button is held.
         */
        virtual void OnMouseMove(
            int _x, int _y,
            const GizmoInteractContext& _ctx,
            ecs::World& _world,
            ecs::Entity _entity
        )
        {
        }

        /**
         * @brief Called when the left mouse button is released.
         */
        virtual void OnMouseRelease(int _x, int _y) {}
    };
}

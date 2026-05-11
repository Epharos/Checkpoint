#pragma once

#include <Common/Plugin/ViewportToolbarContribution.hpp>

#include "Transform.hpp"

namespace cp
{
    /**
     * @brief Shows the Translate / Rotate / Scale toolbar buttons
     * whenever the selected entity owns a Transform component.
     */
    class TransformViewportToolbarContribution final : public IViewportToolbarContribution
    {
    public:
        bool IsApplicable(const ecs::World& world, ecs::Entity entity) const override
        {
            return world.HasComponent<Transform>(entity);
        }

        std::vector<ViewportToolbarButtonDescriptor> GetButtons() const override
        {
            return {
                { "viewport.tool.translate", "Translate", "Switch to Translate mode", true, true  },
                { "viewport.tool.rotate", "Rotate", "Switch to Rotate mode", true, false },
                { "viewport.tool.scale", "Scale", "Switch to Scale mode", true, false },
            };
        }

        void OnButtonTriggered(std::string_view _buttonId) override
        {
            // TODO: notify the gizmo render pass of the active tool.
        }
    };

    inline constexpr cp::ecs::TypeGuid TransformViewportToolbarContributionGuid =
        cp::ecs::MakeTypeGuid("ExamplePlugin.TransformViewportToolbarContribution");
}

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace cp::ecs
{
    class World;
    struct Entity;
}

namespace cp
{
    /**
     * @brief Describes a single button that a contribution wants to add to the viewport toolbar.
     *
     * The editor creates the corresponding IAction from this descriptor at plugin load time.
     * Plugins do not need to pre-register actions; the editor owns creation and lifetime.
     */
    struct ViewportToolbarButtonDescriptor
    {
        std::string id;
        std::string label;
        std::string tooltip;
        bool checkable = false;
        bool checkedByDefault = false;
        bool isActiveTool = true;
    };

    /**
     * @brief Plugin-side interface for extending the viewport toolbar.
     */
    class IViewportToolbarContribution
    {
    public:
        virtual ~IViewportToolbarContribution() = default;

        /**
         * @brief Returns true if this contribution's buttons should be visible for the given entity.
         * Called every time the selection changes.
         */
        virtual bool IsApplicable(const ecs::World& world, ecs::Entity entity) const = 0;

        /**
         * @brief Returns the full descriptor for each button this contribution owns.
         * Called once by the editor at plugin load time to create the IAction objects.
         */
        virtual std::vector<ViewportToolbarButtonDescriptor> GetButtons() const = 0;

        /**
         * @brief Called by the editor when one of this contribution's buttons is triggered.
         * For checkable buttons the editor has already updated SetChecked() before this call.
         * @param _buttonId The id from the ViewportToolbarButtonDescriptor of the triggered button.
         */
        virtual void OnButtonTriggered(std::string_view _buttonId) {}
    };
}

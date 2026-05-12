#pragma once

namespace cp
{
    /**
     * @brief High-level control over the editor viewport camera.
     */
    class IViewportController
    {
    public:
        virtual ~IViewportController() = default;

        /**
         * @brief Moves the editor camera so that the given world-space position
         *        is centred in view, at a reasonable framing distance.
         *        The camera's current orientation (pitch / yaw) is preserved.
         */
        virtual void FocusOn(float _x, float _y, float _z, float _distance) = 0;
    };
}

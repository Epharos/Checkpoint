#pragma once

#include <functional>

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

        /** @brief Translates the camera in its local right/up plane. */
        virtual void Pan(float _rightDelta, float _upDelta) = 0;

        /** @brief Rotates the camera in place (radians). Pitch must be clamped to avoid gimbal flip. */
        virtual void Rotate(float _pitchDelta, float _yawDelta) = 0;

        /** @brief Translates the camera along its forward vector. */
        virtual void MoveForward(float _delta) = 0;

        /** @brief Orbits the camera around a world-space target (radians). */
        virtual void OrbitAround(
            float _targetX,
            float _targetY,
            float _targetZ,
            float _pitchDelta,
            float _yawDelta
        ) = 0;

        /**
         * @brief Registers a callback invoked to resolve the orbit target world position.
         * The callback should return true and fill x/y/z when a valid target exists.
         * Plugins use this to expose Transform-based orbit targets.
         */
        virtual void SetOrbitTargetProvider(std::function<bool(float&, float&, float&)> _provider) = 0;
    };
}

#pragma once

#include <cmath>
#include <optional>

#include <Rendering/GizmoContribution.hpp>

#include <ECS/ECS.hpp>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Components/Transform.hpp"

namespace cp
{
    inline constexpr cp::ecs::TypeGuid TransformGizmoContributionGuid =
        cp::ecs::MakeTypeGuid("ExamplePlugin.TransformGizmoContribution");

    /**
     * @brief Gizmo contribution that renders Translate, Rotate and Scale handles
     * for entities that own a Transform component, and handles mouse interaction.
     *
     * Axis convention
     *   X = red ( 1, 0, 0 )
     *   Y = green ( 0, 1, 0 )
     *   Z = blue ( 0, 0, 1 )
     */
    class TransformGizmoContribution final : public IGizmoContribution
    {
    public:
        bool IsApplicable(const ecs::World& _world, ecs::Entity _entity) const override
        {
            return _world.HasComponent<Transform>(_entity);
        }

        void Render(const GizmoRenderContext& _ctx, const ecs::World& _world, ecs::Entity _entity) override
        {
            const Transform* transform = _world.TryGetComponent<Transform>(_entity);
            if (!transform) return;

            const glm::vec3 origin(transform->x, transform->y, transform->z);
            const float handleLen = ComputeGizmoScale(_ctx.camera, _ctx.viewport, origin);
            const LocalAxes axes = (_ctx.space == GizmoSpace::Local) ? ComputeLocalAxes(*transform) : LocalAxes {};

            if (_ctx.activeTool == "viewport.tool.translate")
                DrawTranslateHandles(_ctx.debugDrawBuffer, origin, handleLen, axes);
            else if (_ctx.activeTool == "viewport.tool.rotate")
                DrawRotateHandles(_ctx.debugDrawBuffer, origin, handleLen, axes);
            else if (_ctx.activeTool == "viewport.tool.scale")
                DrawScaleHandles(_ctx.debugDrawBuffer, origin, handleLen, axes);
        }

        bool OnMousePress(
            int _x, int _y,
            const GizmoInteractContext& _ctx,
            ecs::World& _world,
            ecs::Entity _entity
        ) override
        {
            const Transform* transform = _world.TryGetComponent<Transform>(_entity);
            if (!transform) return false;

            const glm::vec3 origin(transform->x, transform->y, transform->z);
            const float handleLen = ComputeGizmoScale(_ctx.camera, _ctx.viewport, origin);
            const glm::vec2 mousePos(static_cast<float>(_x), static_cast<float>(_y));
            const LocalAxes axes = (_ctx.space == GizmoSpace::Local) ? ComputeLocalAxes(*transform) : LocalAxes{};

            if (_ctx.activeTool == "viewport.tool.rotate")
                return HitTestRotate(mousePos, origin, handleLen, _ctx, axes);
            if (_ctx.activeTool == "viewport.tool.scale")
                return HitTestLinear(mousePos, origin, handleLen, _ctx, DragMode::Scale, axes);

            return HitTestLinear(mousePos, origin, handleLen, _ctx, DragMode::Translate, axes);
        }

        void OnMouseMove(
            int _x, int _y,
            const GizmoInteractContext& _ctx,
            ecs::World& _world, ecs::Entity _entity
        ) override
        {
            if (dragAxis < 0) return;
            Transform* transform = _world.TryGetComponent<Transform>(_entity);
            if (!transform) return;

            const glm::vec2 mousePos(static_cast<float>(_x), static_cast<float>(_y));
            const float screenDelta = glm::dot(mousePos - lastMousePos, screenAxisDir);
            lastMousePos = mousePos;

            switch (dragMode)
            {
            case DragMode::Translate:
            {
                const float worldDelta = screenDelta / dragScale;
                transform->x += dragAxisWorld.x * worldDelta;
                transform->y += dragAxisWorld.y * worldDelta;
                transform->z += dragAxisWorld.z * worldDelta;
                break;
            }
            case DragMode::Scale:
            {
                constexpr float Sensitivity = 0.5f;
                const float delta = (screenDelta / dragScale) * Sensitivity;
                if (dragAxis == 0) transform->scaleX = glm::max(0.001f, transform->scaleX + delta);
                else if (dragAxis == 1) transform->scaleY = glm::max(0.001f, transform->scaleY + delta);
                else transform->scaleZ = glm::max(0.001f, transform->scaleZ + delta);
                break;
            }
            case DragMode::Rotate:
            {
                const float radians = screenDelta * glm::half_pi<float>() / dragScale;

                // Build the current rotation matrix (YXZ convention: Ry * Rx * Rz).
                const glm::mat4 R = glm::eulerAngleYXZ(
                    glm::radians(transform->yaw),
                    glm::radians(transform->pitch),
                    glm::radians(transform->roll));

                const glm::vec3 axis = (dragAxis == 0) ? glm::vec3(1.f, 0.f, 0.f)
                                     : (dragAxis == 1) ? glm::vec3(0.f, 1.f, 0.f)
                                                       : glm::vec3(0.f, 0.f, 1.f);

                const glm::mat4 delta = glm::rotate(glm::mat4(1.f), radians, axis);

                const glm::mat4 R_new = (_ctx.space == GizmoSpace::Local) ? R * delta : delta * R;

                float newYaw, newPitch, newRoll;
                glm::extractEulerAngleYXZ(R_new, newYaw, newPitch, newRoll);
                transform->yaw = glm::degrees(newYaw);
                transform->pitch = glm::degrees(newPitch);
                transform->roll = glm::degrees(newRoll);
                break;
            }
            }
        }

        void OnMouseRelease(int _x, int _y) override
        {
            dragAxis = -1;
        }

    private:
        enum class DragMode { Translate, Scale, Rotate };

        int dragAxis = -1;
        DragMode dragMode = DragMode::Translate;
        glm::vec2 lastMousePos = {};
        glm::vec2 screenAxisDir = {};
        float dragScale = 1.f;
        glm::vec3 dragAxisWorld = {};

        struct LocalAxes
        {
            glm::vec3 x{1.f, 0.f, 0.f};
            glm::vec3 y{0.f, 1.f, 0.f};
            glm::vec3 z{0.f, 0.f, 1.f};
        };

        static LocalAxes ComputeLocalAxes(const Transform& _transform)
        {
            auto rot = glm::mat4(1.0f);
            rot = glm::rotate(rot, glm::radians(_transform.yaw), glm::vec3(0.f, 1.f, 0.f));
            rot = glm::rotate(rot, glm::radians(_transform.pitch), glm::vec3(1.f, 0.f, 0.f));
            rot = glm::rotate(rot, glm::radians(_transform.roll), glm::vec3(0.f, 0.f, 1.f));
            return { glm::vec3(rot[0]), glm::vec3(rot[1]), glm::vec3(rot[2]) };
        }

        static glm::vec4 AxisColor(int _axis)
        {
            if (_axis == 0) return glm::vec4(0.95f, 0.20f, 0.20f, 1.f);
            if (_axis == 1) return glm::vec4(0.20f, 0.90f, 0.20f, 1.f);
            return glm::vec4(0.20f, 0.20f, 0.95f, 1.f);
        }

        static constexpr float GizmoScreenFraction = 0.08f;
        static constexpr float HitThresholdPixels = 8.f;

        static float ComputeGizmoScale(
            const ResolvedCameraData& _camera,
            const Extent2D<uint32_t>& _viewport,
            const glm::vec3& _worldPos
        )
        {
            const float viewDist = glm::length(_camera.worldPosition - _worldPos);
            if (viewDist < 0.001f) return 1.f;
            const float frustumHalfHeight = viewDist * std::tan(glm::radians(_camera.fovYDegrees * 0.5f));
            return frustumHalfHeight * 2.f * GizmoScreenFraction;
        }

        static glm::vec2 ProjectToScreen(
            const glm::vec3& _worldPos,
            const ResolvedCameraData& _camera,
            const Extent2D<uint32_t>& _viewport
        )
        {
            const glm::vec4 clip = _camera.viewProjection * glm::vec4(_worldPos, 1.f);
            if (clip.w <= 0.f) return glm::vec2(-99999.f);
            const glm::vec3 ndc = glm::vec3(clip) / clip.w;
            const float viewportWidth = static_cast<float>(_viewport.x());
            const float viewportHeight = static_cast<float>(_viewport.y());
            return glm::vec2(
                (ndc.x * 0.5f + 0.5f) * viewportWidth,
                (ndc.y * 0.5f + 0.5f) * viewportHeight
            );
        }

        static float PointToSegmentDist2D(const glm::vec2& _p, const glm::vec2& _a, const glm::vec2& _b)
        {
            const glm::vec2 ab = _b - _a;
            const float lenSq = glm::dot(ab, ab);
            if (lenSq < 1e-6f) return glm::length(_p - _a);
            const float t = glm::clamp(glm::dot(_p - _a, ab) / lenSq, 0.f, 1.f);
            return glm::length(_p - (_a + t * ab));
        }

        bool HitTestLinear(
            const glm::vec2& _mousePos,
            const glm::vec3& _origin,
            float _handleLen,
            const GizmoInteractContext& _ctx,
            DragMode _mode,
            const LocalAxes& _axes
        )
        {
            const glm::vec3 axesDirs[3] = { _axes.x, _axes.y, _axes.z };
            for (int axis = 0; axis < 3; ++axis)
            {
                const glm::vec3 axisDir = axesDirs[axis];
                const glm::vec3 target = _origin + axisDir * _handleLen;
                const glm::vec2 screenOrigin = ProjectToScreen(_origin, _ctx.camera, _ctx.viewport);
                const glm::vec2 screenTarget = ProjectToScreen(target, _ctx.camera, _ctx.viewport);

                if (PointToSegmentDist2D(_mousePos, screenOrigin, screenTarget) < HitThresholdPixels)
                {
                    const glm::vec2 axisVec = screenTarget - screenOrigin;
                    const float screenLen = glm::length(axisVec);

                    dragAxis = axis;
                    dragMode = _mode;
                    lastMousePos = _mousePos;
                    screenAxisDir = (screenLen > 0.5f) ? axisVec / screenLen : glm::vec2(1.f, 0.f);
                    dragScale = (screenLen > 0.5f) ? screenLen / _handleLen : 1.f;
                    dragAxisWorld = axisDir;
                    return true;
                }
            }
            return false;
        }

        bool HitTestRotate(
            const glm::vec2& _mousePos,
            const glm::vec3& _origin,
            float _radius,
            const GizmoInteractContext& _ctx,
            const LocalAxes& _axes
        )
        {
            constexpr int SamplesCount = 32;
            constexpr float Step = glm::two_pi<float>() / static_cast<float>(SamplesCount);

            const glm::vec3 b1[3] = { _axes.y, _axes.x, _axes.x };
            const glm::vec3 b2[3] = { _axes.z, _axes.z, _axes.y };

            const glm::vec2 centerScreen = ProjectToScreen(_origin, _ctx.camera, _ctx.viewport);

            int bestAxis = -1;
            float bestDist = HitThresholdPixels;
            glm::vec2 bestPoint = {};
            int bestSampleIndex = 0;

            for (int axis = 0; axis < 3; ++axis)
            {
                const glm::vec3& u = b1[axis];
                const glm::vec3& v = b2[axis];

                for (int i = 0; i < SamplesCount; ++i)
                {
                    const float a = static_cast<float>(i) * Step;
                    const glm::vec3 worldPoint = _origin + (u * std::cos(a) + v * std::sin(a)) * _radius;
                    const glm::vec2 screenPoint = ProjectToScreen(worldPoint, _ctx.camera, _ctx.viewport);

                    const float dist = glm::length(_mousePos - screenPoint);
                    if (dist < bestDist)
                    {
                        bestDist = dist;
                        bestAxis = axis;
                        bestPoint = screenPoint;
                        bestSampleIndex = i;
                    }
                }
            }

            if (bestAxis < 0) return false;

            {
                const glm::vec3& u = b1[bestAxis];
                const glm::vec3& v = b2[bestAxis];
                const int nextIndex = (bestSampleIndex + 1) % SamplesCount;
                const float nextAngle = static_cast<float>(nextIndex) * Step;
                const glm::vec3 nextWorld = _origin + (u * std::cos(nextAngle) + v * std::sin(nextAngle)) * _radius;
                const glm::vec2 nextScreen = ProjectToScreen(nextWorld, _ctx.camera, _ctx.viewport);
                const glm::vec2 tangent = nextScreen - bestPoint;
                const float tangentLen = glm::length(tangent);

                screenAxisDir = (tangentLen > 0.5f) ? tangent / tangentLen : glm::vec2(1.f, 0.f);
            }

            const glm::vec2 radialVec = bestPoint - centerScreen;
            const float screenRad = glm::length(radialVec);

            dragAxis = bestAxis;
            dragMode = DragMode::Rotate;
            lastMousePos = _mousePos;
            dragScale = (screenRad > 0.5f) ? screenRad : 50.f;
            return true;
        }

        static void DrawTranslateHandles(
            DebugDrawBuffer& _buffer,
            const glm::vec3& _origin,
            float _handleLen,
            const LocalAxes& _axes
        )
        {
            const glm::vec3 axesDirs[3] = { _axes.x, _axes.y, _axes.z };
            for (int axis = 0; axis < 3; ++axis)
            {
                const glm::vec3 axisDir = axesDirs[axis];
                const glm::vec4 color = AxisColor(axis);
                const glm::vec3 tip = _origin + axisDir * _handleLen;

                _buffer.AddLine(_origin, tip, color);

                const float headLen = _handleLen * 0.18f;
                const glm::vec3 perpendicularReference = axesDirs[(axis + 1) % 3];
                const glm::vec3 perpendicular1 = glm::normalize(glm::cross(axisDir, perpendicularReference)) * headLen;
                const glm::vec3 perpendicular2 = glm::normalize(glm::cross(axisDir, perpendicular1)) * headLen;
                const glm::vec3 base = tip - axisDir * headLen;
                _buffer.AddLine(tip, base + perpendicular1, color);
                _buffer.AddLine(tip, base - perpendicular1, color);
                _buffer.AddLine(tip, base + perpendicular2, color);
                _buffer.AddLine(tip, base - perpendicular2, color);
            }
        }

        static void DrawRotateHandles(
            DebugDrawBuffer& _buffer,
            const glm::vec3& _origin,
            const float _radius,
            const LocalAxes& _axes
        )
        {
            constexpr int SegmentsCount = 32;
            constexpr float Step = glm::two_pi<float>() / static_cast<float>(SegmentsCount);

            const glm::vec3 b1[3] = { _axes.y, _axes.x, _axes.x };
            const glm::vec3 b2[3] = { _axes.z, _axes.z, _axes.y };

            for (int axis = 0; axis < 3; ++axis)
            {
                const glm::vec4 color = AxisColor(axis);
                const glm::vec3& u = b1[axis];
                const glm::vec3& v = b2[axis];

                for (int i = 0; i < SegmentsCount; ++i)
                {
                    const float a0 = static_cast<float>(i) * Step;
                    const float a1 = static_cast<float>(i + 1) * Step;

                    const glm::vec3 p0 = _origin + (u * std::cos(a0) + v * std::sin(a0)) * _radius;
                    const glm::vec3 p1 = _origin + (u * std::cos(a1) + v * std::sin(a1)) * _radius;
                    _buffer.AddLine(p0, p1, color);
                }
            }
        }

        static void DrawScaleHandles(
            DebugDrawBuffer& _buffer,
            const glm::vec3& _origin,
            float _handleLen,
            const LocalAxes& _axes
        )
        {
            const glm::vec3 axesDirs[3] = { _axes.x, _axes.y, _axes.z };
            for (int axis = 0; axis < 3; ++axis)
            {
                const glm::vec3 axisDir = axesDirs[axis];
                const glm::vec4 color = AxisColor(axis);
                const glm::vec3 tip = _origin + axisDir * _handleLen;

                _buffer.AddLine(_origin, tip, color);

                const float h = _handleLen * 0.07f;
                const glm::vec3 perpendicularReference = axesDirs[(axis + 1) % 3];
                const glm::vec3 perpendicular1 = glm::normalize(glm::cross(axisDir, perpendicularReference)) * h;
                const glm::vec3 perpendicular2 = glm::normalize(glm::cross(axisDir, perpendicular1)) * h;

                const glm::vec3 c00 = tip + perpendicular1 + perpendicular2;
                const glm::vec3 c01 = tip + perpendicular1 - perpendicular2;
                const glm::vec3 c10 = tip - perpendicular1 + perpendicular2;
                const glm::vec3 c11 = tip - perpendicular1 - perpendicular2;
                _buffer.AddLine(c00, c01, color);
                _buffer.AddLine(c01, c11, color);
                _buffer.AddLine(c11, c10, color);
                _buffer.AddLine(c10, c00, color);
            }
        }
    };
}

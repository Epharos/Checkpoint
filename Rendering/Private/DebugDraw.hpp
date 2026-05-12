#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace cp
{
    /**
     * @brief A single debug line segment with RGBA color.
     */
    struct DebugLine
    {
        glm::vec3 start;
        glm::vec3 end;
        glm::vec4 color;
    };

    /**
     * @brief CPU-side buffer of debug line segments for a single frame.
     *
     * Populated by editor systems (e.g. TransformGizmoContribution) before BeginFrame()
     * and consumed by DebugDrawPass during Execute().
     * Cleared automatically by Renderer::EndFrame().
     */
    class DebugDrawBuffer
    {
    public:
        void AddLine(const glm::vec3& _start, const glm::vec3& _end, const glm::vec4& _color)
        {
            lines.push_back({ _start, _end, _color });
        }

        void AddLine(const glm::vec3& _start, const glm::vec3& _end, const glm::vec3& _color)
        {
            lines.push_back({ _start, _end, glm::vec4(_color, 1.0f) });
        }

        void Clear() { lines.clear(); }

        [[nodiscard]] const std::vector<DebugLine>& GetLines() const { return lines; }
        [[nodiscard]] bool IsEmpty() const { return lines.empty(); }

    private:
        std::vector<DebugLine> lines;
    };
}

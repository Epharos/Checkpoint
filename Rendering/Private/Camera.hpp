#pragma once

#include <glm/glm.hpp>

namespace cp
{
    /**
     * @brief Free-flying camera owned by the Renderer when running in Editor mode.
     *
     * The renderer computes a ResolvedCameraData from this struct at the start of
     * each frame and injects the result into every render pass via
     * RenderPassExecutionContext::camera.
     */
    struct EditorCamera
    {
        glm::vec3 position = { 0.f, 0.f, 5.f };
        float pitch = 0.f;
        float yaw = 0.f;
        float fovYDegrees = 60.f;
        float nearPlane = 0.1f;
        float farPlane = 1000.f;
    };

    /**
     * @brief Camera matrices and parameters resolved once per frame.
     *
     * In Editor mode the renderer fills this from EditorCamera.
     * In Runtime/DevelopmentBuild mode isValid is false; render passes are
     * responsible for finding a primary camera in the ECS world themselves.
     */
    struct ResolvedCameraData
    {
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        glm::mat4 viewProjection = glm::mat4(1.0f);
        glm::vec3 worldPosition = glm::vec3(0.0f);
        float nearPlane = 0.1f;
        float farPlane = 1000.f;
        float fovYDegrees = 60.f;

        bool isValid = false;
    };
}

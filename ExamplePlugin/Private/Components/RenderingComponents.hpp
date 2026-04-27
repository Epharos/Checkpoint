#pragma once

#include <Resources/Mesh.hpp>
#include <Resources/AssetHandle.hpp>

#include <string>

namespace cp
{
    struct Transform3D
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        float pitch = 0.0f;
        float yaw = 0.0f;
        float roll = 0.0f;

        float scaleX = 1.0f;
        float scaleY = 1.0f;
        float scaleZ = 1.0f;
    };

    struct Camera
    {
        float fovYDegrees = 60.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        bool isPrimary = true;
        bool enabled = true;
    };

    struct MeshRenderer
    {
        AssetHandle<Mesh> meshId {};
        bool visible = true;
    };
}

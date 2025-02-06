#pragma once

#include "../../pch.hpp"

enum class CameraType
{
	Perspective,
	Orthographic
};

struct CameraUBO
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 viewProjection;
	glm::vec2 nearFar;
};

struct Camera
{
	CameraType type = CameraType::Perspective;

	CameraUBO cameraUBO;

	// ORTHOGRAPHIC DATA
	float left = -1.0f;
	float right = 1.0f;
	float bottom = -1.0f;
	float top = 1.0f;

	// PERSPECTIVE DATA
	float fov = 70.0f; // degrees
	float aspectRatio = 16.0f / 9.0f;

	// COMMON DATA
	float near = 0.1f;
	float far = 100.0f;

	bool dirty = true;

	bool Update(const Transform& _transform);

private:
	void UpdateProjection();
	void UpdateView(const Transform& _transform);
};
#pragma once

#include "../../pch.hpp"

class Controller : public ECS::System
{
protected:
	Render::Camera* affectedCamera;
	GLFWwindow* window;

	int windowWidth;
	int windowHeight;

	float sensitivity;
	float moveSpeed;
	float pitch, yaw, roll;

public:
	Controller(Render::Camera* _camera, GLFWwindow* _window, const float& _moveSpeed = 20.f, const float& _sensitivity = 0.5f);
	virtual void Update(ECS::ComponentManager& _componentManager, const float& _dt);
};
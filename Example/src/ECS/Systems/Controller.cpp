#include "pch.hpp"
#include "Controller.hpp"

Controller::Controller(Render::Camera* _camera, GLFWwindow* _window, const float& _moveSpeed, const float& _sensitivity)
{
	this->affectedCamera = _camera;
	this->window = _window;

	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetWindowSize(_window, &windowWidth, &windowHeight);
	glfwSetCursorPos(_window, windowWidth / 2.f, windowHeight / 2.f);

	this->moveSpeed = _moveSpeed;
	this->sensitivity = _sensitivity;
}

void Controller::Update(ECS::ComponentManager& _componentManager, const float& _dt)
{
	glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? affectedCamera->Translate(affectedCamera->GetForward() * _dt * moveSpeed) : void();
	glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? affectedCamera->Translate(-affectedCamera->GetForward() * _dt * moveSpeed) : void();
	glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? affectedCamera->Translate(-affectedCamera->GetRight() * _dt * moveSpeed) : void();
	glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? affectedCamera->Translate(affectedCamera->GetRight() * _dt * moveSpeed) : void();
	glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ? affectedCamera->Translate(VEC3_UP * _dt * moveSpeed) : void();
	glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? affectedCamera->Translate(-VEC3_UP * _dt * moveSpeed) : void();

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	double deltaX = mouseX - windowWidth / 2.f;
	double deltaY = mouseY - windowHeight / 2.f;

	pitch += deltaY * sensitivity * _dt;
	yaw += deltaX * sensitivity * _dt;

	affectedCamera->SetRotationEuler(glm::vec3(pitch, -yaw, roll));

	glfwSetCursorPos(window, windowWidth / 2.f, windowHeight / 2.f);
}

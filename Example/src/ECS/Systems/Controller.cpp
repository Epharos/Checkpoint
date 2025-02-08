#include "pch.hpp"
#include "Controller.hpp"

Controller::Controller(GLFWwindow* _window)
{
	this->window = _window;

	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetWindowSize(_window, &windowWidth, &windowHeight);
	glfwSetCursorPos(_window, windowWidth / 2.f, windowHeight / 2.f);
}

void Controller::Update(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager, const float& _dt)
{
	_componentManager.ForEachArchetype<CharacterController, Transform>([&](Entity _entity, CharacterController& _controller, Transform& _transform)
	{
		glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? _transform.Translate(_transform.GetForward() * _dt * _controller.speed) : void();
		glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? _transform.Translate(-_transform.GetForward() * _dt * _controller.speed) : void();
		glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? _transform.Translate(_transform.GetRight() * _dt * _controller.speed) : void();
		glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? _transform.Translate(-_transform.GetRight() * _dt * _controller.speed) : void();
		glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ? _transform.Translate(VEC3_UP * _dt * _controller.speed) : void();
		glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? _transform.Translate(-VEC3_UP * _dt * _controller.speed) : void();

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		double deltaX = mouseX - windowWidth / 2.f;
		double deltaY = mouseY - windowHeight / 2.f;

		_controller.pitch += deltaY * _controller.sensitivity * _dt;
		_controller.yaw -= deltaX * _controller.sensitivity * _dt;

		_controller.pitch = glm::clamp(_controller.pitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

		_transform.SetRotation(glm::vec3(_controller.pitch, _controller.yaw, _controller.roll));

		glfwSetCursorPos(window, windowWidth / 2.f, windowHeight / 2.f);
	});

	_componentManager.ForEachArchetype<CameraFollow, Transform>([&](Entity _entity, CameraFollow& _camera, Transform& _transform)
	{
		auto& targetTransform = _componentManager.GetComponent<Transform>(_camera.cameraEntity);

		targetTransform.SetPosition(_transform.GetPosition());
		targetTransform.SetRotation(_transform.GetRotation());
	});
}

void Controller::Cleanup()
{

}

#pragma once

#include "../../pch.hpp"

class Controller : public ECS::System
{
protected:
	GLFWwindow* window;

	int windowWidth;
	int windowHeight;

public:
	Controller(GLFWwindow* _window);
	virtual void Update(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager, const float& _dt);
	virtual void Cleanup() override;
};
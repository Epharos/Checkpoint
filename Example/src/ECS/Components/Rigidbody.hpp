#pragma once

#include "../../pch.hpp"

struct Rigidbody
{
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);

	float mass = 1.0f;
	float drag = 0.0f;
	float angularDrag = 0.0f;

	bool useGravity = true;
	bool isKinematic = false;

	glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
};
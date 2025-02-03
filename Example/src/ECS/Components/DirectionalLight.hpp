#pragma once

#include "../../pch.hpp"

struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;

	float* splits;
	glm::mat4* projectionMatrix;
	glm::mat4* viewMatrix;
	glm::mat4* viewProjections;

	uint8_t cascadeCount;
	uint16_t shadowMapSize;

	DirectionalLight(const glm::vec3& _direction, const glm::vec3& _color, const float& _intensity, const uint8_t& _cascadeCount, const uint16_t& _shadowMapSize) : 
		direction(_direction), color(_color), intensity(_intensity), cascadeCount(_cascadeCount), shadowMapSize(_shadowMapSize),
		splits(nullptr), projectionMatrix(nullptr), viewMatrix(nullptr), viewProjections(nullptr) {}
};
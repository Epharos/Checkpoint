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

	DirectionalLight() : direction(glm::vec3(0.0f)), color(glm::vec3(1.0f)), intensity(1.0f), cascadeCount(4), shadowMapSize(2048),
		splits(nullptr), projectionMatrix(nullptr), viewMatrix(nullptr), viewProjections(nullptr) {}

	DirectionalLight(const glm::vec3& _direction, const glm::vec3& _color, const float& _intensity, const uint8_t& _cascadeCount, const uint16_t& _shadowMapSize) : 
		direction(_direction), color(_color), intensity(_intensity), cascadeCount(_cascadeCount), shadowMapSize(_shadowMapSize),
		splits(nullptr), projectionMatrix(nullptr), viewMatrix(nullptr), viewProjections(nullptr) {}
};
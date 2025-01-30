#pragma once

#include "../../pch.hpp"

struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;

	vk::Image shadowMap;

	uint8_t cascadeCount;
	uint16_t shadowMapSize;

	DirectionalLight(const glm::vec3& _direction, const glm::vec3& _color, const float& _intensity, const uint8_t& _cascadeCount, const uint16_t& _shadowMapSize) : 
		direction(_direction), color(_color), intensity(_intensity), cascadeCount(_cascadeCount), shadowMapSize(_shadowMapSize) {}
};
#pragma once

#include "../../pch.hpp"

struct CharacterController
{
	float speed;
	float sensitivity;

	float pitch, yaw, roll;

	CharacterController(float _speed = 5.0f, float _sensitivity = 5.0f)
		: speed(_speed), sensitivity(_sensitivity) 
	{
		pitch = 0.0f;
		yaw = 0.0f;
		roll = 0.0f;
	}
};
#include "pch.hpp"
#include "Transform.hpp"

void Transform::Translate(const glm::vec3& _translation)
{
	position += _translation;
	dirty = true;
}

void Transform::SetPosition(const glm::vec3& _position)
{
	position = _position;
	dirty = true;
}

void Transform::Rotate(const glm::quat& _rotation)
{
	rotation = _rotation * rotation;
	dirty = true;
}

void Transform::SetRotation(const glm::quat& _rotation)
{
	rotation = _rotation;
	dirty = true;
}

void Transform::SetScale(const glm::vec3& _scale)
{
	scale = _scale;
	dirty = true;
}

const glm::mat4 Transform::GetModelMatrix()
{
	if (dirty)
	{
		matrix = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
		dirty = false;
	}

	return matrix;
}
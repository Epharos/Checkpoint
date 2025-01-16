#include "pch.hpp"
#include "Transform.hpp"

void Transform::UpdateMatrix()
{
	if (!dirty) return; // Early return if the matrix is up to date

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::toMat4(rotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

	matrix = translationMatrix * rotationMatrix * scaleMatrix;
	normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));

	dirty = false;
}

Transform::Transform(glm::vec3 _position, glm::quat _rotation, glm::vec3 _scale)
{
	position = _position;
	rotation = _rotation;
	scale = _scale;
	dirty = true;
}

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
	UpdateMatrix();
	return matrix;
}

const glm::mat3 Transform::GetNormalMatrix()
{
	UpdateMatrix();
	return normalMatrix;
}

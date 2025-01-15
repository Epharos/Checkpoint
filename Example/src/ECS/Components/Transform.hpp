#pragma once

#include "../../pch.hpp"

struct Transform
{
protected:
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	bool dirty = true;
	glm::mat4 matrix = glm::mat4(1.0f);
	glm::mat3 normalMatrix = glm::mat3(1.0f);

	void UpdateMatrix();

public:
	Transform(glm::vec3 _position = glm::vec3(0.0f), glm::quat _rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3 _scale = glm::vec3(1.0f));

	void Translate(const glm::vec3& _translation);

	void SetPosition(const glm::vec3& _position);

	void Rotate(const glm::quat& _rotation);

	void SetRotation(const glm::quat& _rotation);

	void SetScale(const glm::vec3& _scale);

	inline constexpr const glm::vec3 GetPosition() const { return position; }
	inline constexpr const glm::quat GetRotation() const { return rotation; }
	inline constexpr const glm::vec3 GetScale() const { return scale; }

	const glm::mat4 GetModelMatrix();
	const glm::mat3 GetNormalMatrix();
};
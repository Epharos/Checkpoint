#include "pch.hpp"
#include "Camera.hpp"

namespace Render
{
	Camera::Camera(Context::VulkanContext* _context)
	{
		context = _context;

		position = glm::vec3(0.0f); // Default position
		rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Default rotation

		viewMatrix = glm::mat4(1.0f); // Identity matrix
		projectionMatrix = glm::mat4(1.0f); // Identity matrix

		uboBuffer = Helper::Memory::CreateBuffer(context->GetDevice(), context->GetPhysicalDevice(), sizeof(CameraUBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uboBufferMemory);

		SetPerspective(70.f, context->GetPlatform()->GetAspectRatio(), 0.1f, 300.f);

		dirty = true;
		UpdateUniformBuffer();
	}

	Camera::~Camera()
	{
		context->GetDevice().destroyBuffer(uboBuffer);
		context->GetDevice().freeMemory(uboBufferMemory);
	}

	void Camera::SetPosition(const glm::vec3& _position)
	{
		position = _position;
		dirty = true;
	}

	void Camera::SetRotation(const glm::quat& _rotation)
	{
		rotation = glm::normalize(_rotation);
		dirty = true;
	}

	void Camera::SetRotationEuler(const glm::vec3& _rotation)
	{
		rotation = glm::normalize(glm::quat(_rotation));
		dirty = true;
	}

	void Camera::LookAt(const glm::vec3& _target, const glm::vec3& _up)
	{
		glm::vec3 forward = glm::normalize(_target - position);
		glm::vec3 fallback = _up;

		if (glm::abs(glm::dot(forward, _up)) > 0.9999f)
		{
			fallback = VEC3_RIGHT;

			if (glm::abs(forward.x) > 0.9999f)
			{
				fallback = VEC3_FORWARD;
			}
		}	

		glm::vec3 right = glm::normalize(glm::cross(fallback, forward));
		glm::vec3 up = glm::cross(forward, right);

		glm::mat3 lookAtMatrix = glm::mat3(right, up, forward);

		rotation = glm::quat_cast(lookAtMatrix);
		dirty = true;
	}

	void Camera::SetPerspective(float _fov, float _aspectRatio, float _near, float _far)
	{
		projectionMatrix = glm::perspectiveRH_ZO(glm::radians(_fov), _aspectRatio, _near, _far);
		projectionMatrix[1][1] *= -1; // Flip the y-axis
		dirty = true;
	}

	void Camera::SetOrthographic(float _left, float _right, float _bottom, float _top, float _near, float _far)
	{
		projectionMatrix = glm::orthoRH_ZO(_left, _right, _bottom, _top, _near, _far);
		projectionMatrix[1][1] *= -1; // Flip the y-axis
		dirty = true;
	}

	void Camera::Translate(const glm::vec3& _translation)
	{
		position += _translation;
		dirty = true;
	}

	void Camera::Rotate(const glm::quat& _rotation)
	{
		rotation = glm::normalize(_rotation * rotation);
		dirty = true;
	}

	void Camera::Rotate(const glm::vec3& _rotation)
	{
		rotation = glm::normalize(glm::quat(_rotation) * rotation);
		dirty = true;
	}

	void Camera::UpdateUniformBuffer()
	{
		if (dirty)
		{
			viewMatrix = glm::lookAtRH(position, position + rotation * VEC3_FORWARD, VEC3_UP);
			ubo.viewProjectionMatrix = projectionMatrix * viewMatrix;
			Helper::Memory::MapMemory(context->GetDevice(), uboBufferMemory, sizeof(CameraUBO), &ubo);
			dirty = false;
		}
	}
}
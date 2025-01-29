#pragma once

#include "../../pch.hpp"
#include "../../Context/VulkanContext.hpp"

namespace Render
{
	struct CameraUBO
	{
		glm::mat4 viewProjectionMatrix;
	};

	class Camera
	{
	public:
		Camera(Context::VulkanContext* _context);
		~Camera();

		void SetPosition(const glm::vec3& _position);
		void SetRotation(const glm::quat& _rotation);
		void SetRotationEuler(const glm::vec3& _rotation);
		void LookAt(const glm::vec3& _target, const glm::vec3& _up = VEC3_UP);

		void SetPerspective(float _fov, float _aspectRatio, float _near, float _far);
		void SetOrthographic(float _left, float _right, float _bottom, float _top, float _near, float _far);

		inline constexpr glm::vec3& GetPosition() { return position; }
		inline constexpr glm::quat& GetRotation() { return rotation; }
		inline glm::vec3 GetRotationEuler() const { return glm::eulerAngles(rotation); }

		inline constexpr glm::mat4& GetViewMatrix() { return viewMatrix; }
		inline constexpr glm::mat4& GetProjectionMatrix() { return projectionMatrix; }
		inline constexpr glm::mat4 GetViewProjectionMatrix() const { return projectionMatrix * viewMatrix; }

		inline constexpr vk::Buffer& GetUBOBuffer() { return uboBuffer; }
		inline constexpr vk::DeviceMemory& GetUBOBufferMemory() { return uboBufferMemory; }

		inline glm::vec3 GetForward() const { return glm::normalize(rotation * VEC3_FORWARD); }
		inline glm::vec3 GetRight() const { return glm::normalize(glm::cross(GetUp(), GetForward())); }
		inline glm::vec3 GetUp() const { return glm::normalize(rotation * VEC3_UP); }

		void Translate(const glm::vec3& _translation);
		void Rotate(const glm::quat& _rotation);
		void Rotate(const glm::vec3& _rotation);

		void UpdateUniformBuffer();

	private:
		Context::VulkanContext* context;

		glm::vec3 position;
		glm::quat rotation;

		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;

		bool dirty = false;

		CameraUBO ubo;

		vk::Buffer uboBuffer;
		vk::DeviceMemory uboBufferMemory;
	};
}
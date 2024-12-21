#pragma once

#include "../../pch.hpp"
#include "../../Context/VulkanContext.hpp"

namespace Render
{
	struct CameraUBO
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	class Camera
	{
	public:
		Camera(Context::VulkanContext* _context);
		~Camera();

		inline void SetPosition(const glm::vec3& _position) { position = _position; }
		inline void SetRotation(const glm::quat& _rotation) { rotation = glm::normalize(_rotation); }
		inline void SetRotationEuler(const glm::vec3& _rotation) { rotation = glm::quat(_rotation); }

		inline void SetPerspective(float _fov, float _aspectRatio, float _near, float _far) { ubo.projectionMatrix = glm::perspectiveRH_ZO(glm::radians(_fov), _aspectRatio, _near, _far); }
		inline void SetOrthographic(float _left, float _right, float _bottom, float _top, float _near, float _far) { ubo.projectionMatrix = glm::orthoRH_ZO(_left, _right, _bottom, _top, _near, _far); }

		inline constexpr glm::vec3& GetPosition() { return position; }
		inline constexpr glm::quat& GetRotation() { return rotation; }
		inline glm::vec3 GetRotationEuler() const { return glm::eulerAngles(rotation); }

		inline constexpr glm::mat4& GetViewMatrix() { return ubo.viewMatrix; }
		inline constexpr glm::mat4& GetProjectionMatrix() { return ubo.projectionMatrix; }

		inline constexpr vk::Buffer& GetUBOBuffer() { return uboBuffer; }
		inline constexpr vk::DeviceMemory& GetUBOBufferMemory() { return uboBufferMemory; }

		void Translate(const glm::vec3& _translation);
		void Rotate(const glm::quat& _rotation);
		void Rotate(const glm::vec3& _rotation);

		void UpdateUniformBuffer(); //TODO : Call this function only once per frame before rendering, adding a dirty flag to check if the UBO needs to be updated

	private:
		Context::VulkanContext* context;

		glm::vec3 position;
		glm::quat rotation;

		CameraUBO ubo;

		vk::Buffer uboBuffer;
		vk::DeviceMemory uboBufferMemory;
	};
}
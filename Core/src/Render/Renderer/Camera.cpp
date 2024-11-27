#include "pch.hpp"
#include "Camera.hpp"

namespace Render
{
	Camera::Camera(Context::VulkanContext* _context)
	{
		context = _context;

		position = glm::vec3(0.0f); // Default position
		rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Default rotation

		ubo.viewMatrix = glm::mat4(1.0f); // Identity matrix
		ubo.projectionMatrix = glm::mat4(1.0f); // Identity matrix

		uboBuffer = Helper::Memory::CreateBuffer(context->GetDevice(), context->GetPhysicalDevice(), sizeof(CameraUBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uboBufferMemory);

		SetPerspective(glm::radians(70.f), context->GetPlatform()->GetAspectRatio(), 0.1f, 1000.f);

		UpdateUniformBuffer();
	}

	Camera::~Camera()
	{
		context->GetDevice().destroyBuffer(uboBuffer);
		context->GetDevice().freeMemory(uboBufferMemory);
	}

	void Camera::Translate(const glm::vec3& _translation)
	{
		position += _translation;
		UpdateUniformBuffer();
	}

	void Camera::Rotate(const glm::quat& _rotation)
	{
		rotation = glm::normalize(_rotation * rotation);
		UpdateUniformBuffer();
	}

	void Camera::Rotate(const glm::vec3& _rotation)
	{
		rotation = glm::normalize(glm::quat(_rotation) * rotation);
		UpdateUniformBuffer();
	}

	void Camera::UpdateUniformBuffer()
	{
		ubo.viewMatrix = glm::lookAtRH(position, position + (rotation * VEC3_FORWARD), glm::vec3(0.0f, 1.0f, 0.0f));
		void* data = context->GetDevice().mapMemory(uboBufferMemory, 0, sizeof(CameraUBO));
		memcpy(data, &ubo, sizeof(CameraUBO));
		context->GetDevice().unmapMemory(uboBufferMemory);
	}
}
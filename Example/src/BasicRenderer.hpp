#pragma once

#include "pch.hpp"

struct SunLight
{
	glm::mat4 viewProjectionMatrix;
	glm::vec4 lightDirection;
	glm::vec4 lightColor;
	uint8_t cascadeCount;
};

struct ShadowMapCascade
{
	glm::mat4 viewProjectionMatrix;
	uint8_t index;
};

class BasicRenderer : public Render::Renderer
{
protected:
	vk::Buffer instancedBuffer;
	vk::DeviceMemory instancedBufferMemory;

	SunLight sunLight;
	ShadowMapCascade* shadowMapCascades;
	vk::Buffer sunLightBuffer;
	vk::DeviceMemory sunLightBufferMemory;
	vk::Buffer shadowMapCascadesBuffer;
	vk::DeviceMemory shadowMapCascadesBufferMemory;

	Render::Camera* directionnalLight;
	Render::RenderTarget* shadowMapRT;

	const uint32_t MAX_RENDERABLE_ENTITIES = 1000;

	vk::RenderPass shadowMapRenderPass;

	void CreateMainRenderPass() override;
	void CreateRenderPasses() override;

	void RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups) override;

	void SetupPipelines() override;

public:
	BasicRenderer(Context::VulkanContext* _context, const uint32_t& _maxRenderableEntities = 10000);
	~BasicRenderer();

	void UpdateRenderCameraBuffer(const vk::Buffer& _buffer);
	void SetupDirectionalLight(const vk::Extent2D _extent, const glm::vec4& _color, const glm::vec3& _direction, const uint32_t& _cascadeCount);
	void UpdateDirectionalLight(const glm::mat4* _lightViewProj);

	virtual void Cleanup() override;

	//inline virtual constexpr Render::Camera* GetMainCamera() override { return directionnalLight; }
};
#pragma once

#include "pch.hpp"

#define MAX_CASCADE_COUNT 8

struct SunLight
{
	glm::vec4 lightDirection;
	glm::vec4 lightColor;
	uint32_t cascadeCount;
};

struct alignas(16) ShadowMapCascades
{
	glm::mat4 viewProjectionMatrix[MAX_CASCADE_COUNT];
	alignas(16) float splitDepth[MAX_CASCADE_COUNT];
};

class BasicRenderer : public Render::Renderer
{
protected:
	vk::Buffer instancedBuffer;
	vk::DeviceMemory instancedBufferMemory;

	SunLight sunLight;
	ShadowMapCascades shadowMapCascades;
	vk::Buffer sunLightBuffer;
	vk::DeviceMemory sunLightBufferMemory;
	vk::Buffer shadowMapCascadesBuffer;
	vk::DeviceMemory shadowMapCascadesBufferMemory;

	Render::RenderTarget* shadowMapRT;
	uint32_t shadowMapSize;

	const uint32_t MAX_RENDERABLE_ENTITIES = 1000;

	vk::RenderPass shadowMapRenderPass;

	void CreateMainRenderPass() override;
	void CreateRenderPasses() override;

	void RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups) override;

	void SetupPipelines() override;

public:
	BasicRenderer(Context::VulkanContext* _context, const uint32_t& _maxRenderableEntities = 1000);
	~BasicRenderer();

	void UpdateRenderCameraBuffer(const vk::Buffer& _buffer);
	void SetupDirectionalLight(const vk::Extent2D _extent, const glm::vec4& _color, const glm::vec3& _direction, const uint32_t& _cascadeCount, const float* _splits);
	void UpdateDirectionalLight(const glm::mat4* _lightViewProj);

	virtual void Cleanup() override;

	//inline virtual constexpr Render::Camera* GetMainCamera() override { return directionnalLight; }
};
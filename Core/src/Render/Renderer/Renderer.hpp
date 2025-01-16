#pragma once

#include "../../pch.hpp"
#include "../../Context/VulkanContext.hpp"
#include "../Setup/Swapchain.hpp"
#include "RenderCommand.hpp"

#include "../Pipeline/PipelinesManager.hpp"
#include "../Pipeline/LayoutsManager.hpp"
#include "../Pipeline/SetLayoutsManager.hpp"
#include "../Pipeline/DescriptorSetManager.hpp"

#include "Camera.hpp"

namespace Render
{
	struct TransformData
	{
		glm::mat4 modelMatrix;
		glm::mat4 normalMatrix;
	};

	struct InstanceGroup
	{
		Resource::Mesh* mesh;
		std::vector<TransformData> transforms;
		uint32_t instanceOffset = 0;
	};

	class Renderer
	{
	protected:
		Context::VulkanContext* context;
		Swapchain* swapchain;

		vk::RenderPass mainRenderPass;
		uint32_t subpassCount = -1;

		Camera* mainCamera;

		virtual void SetupPipelines() = 0;

		virtual void CreateMainRenderPass() = 0;
		virtual void CreateRenderPasses();

		virtual uint32_t PrepareFrame();
		virtual void SubmitFrame();
		virtual void PresentFrame(uint32_t _index);
		virtual void EndFrame();

		virtual void RenderFrame(const std::vector<InstanceGroup>& _instanceGroups) = 0;

	public:
		Renderer() = default;
		virtual ~Renderer();

		virtual void Build(Context::VulkanContext* _context);

		virtual void Cleanup();
		
		virtual void Render(const std::vector<InstanceGroup>& _instanceGroups);

		inline constexpr const uint32_t GetSubpassCount() const { return subpassCount; }

		
	};
}
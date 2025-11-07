#pragma once

#include "../../pch.hpp"
#include "../../Context/VulkanContext.hpp"
#include "../Setup/Swapchain.hpp"

#include "../Pipeline/PipelinesManager.hpp"
#include "../Pipeline/LayoutsManager.hpp"
#include "../Pipeline/SetLayoutsManager.hpp"
#include "../Pipeline/DescriptorSetManager.hpp"

#include "Renderpass.hpp"

#include "../../Resources/Material.hpp"
#include "../../Resources/MaterialInstance.hpp"
#include "../../Resources/Mesh.hpp"

#include "Camera.hpp"

namespace cp
{
	struct TransformData
	{
		glm::mat4 modelMatrix;
		glm::mat4 normalMatrix;
	};

	struct InstanceGroup
	{
		cp::Material* material;
		cp::MaterialInstance* materialInstance;
		cp::Mesh* mesh;
		std::vector<TransformData> transforms;
		uint32_t instanceOffset = 0;
	};

	class Renderer
	{
	protected:
		cp::VulkanContext* context = nullptr;
		Platform* platform;
		vk::SurfaceKHR surface = VK_NULL_HANDLE;
		Swapchain* swapchain = nullptr;

		vk::RenderPass mainRenderPass = VK_NULL_HANDLE;
		uint32_t subpassCount = -1;

		std::unordered_map<std::string, Renderpass> renderPasses;

		void SetupSurface(Platform* _platform);
		virtual void SetupPipelines() = 0;

		virtual void CreateMainRenderPass() = 0;
		virtual void CreateRenderPasses();

		virtual uint32_t PrepareFrame();
		virtual void SubmitFrame();
		virtual void PresentFrame(uint32_t _index);
		virtual void EndFrame();

		virtual void RenderFrame(const std::vector<InstanceGroup>& _instanceGroups) = 0;

	public:
		Renderer(cp::VulkanContext* _context);
		virtual ~Renderer();

		virtual void Build();

		virtual void Cleanup();
		
		virtual void Render(const std::vector<InstanceGroup>& _instanceGroups);

		inline constexpr const uint32_t GetSubpassCount() const { return subpassCount; }
		inline constexpr cp::VulkanContext* GetContext() { return context; }

		Renderpass& RegisterRenderPass(const std::string& _name);
		Renderpass& GetRenderPass(const std::string& _name);
		std::vector<std::string> GetRenderPassNames();

		inline std::unordered_map<std::string, Renderpass>& GetRenderPasses() { return renderPasses; }
		
	};
}
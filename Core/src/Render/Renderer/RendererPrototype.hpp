#pragma once

#include "../../pch.hpp"
#include "../../Context/VulkanContext.hpp"

#include "Renderpass.hpp"

#include "../../Resources/Material.hpp"
#include "../../Resources/MaterialInstance.hpp"
#include "../../Resources/Mesh.hpp"

#include "../Setup/Swapchain.hpp"

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
		//uint32_t instanceOffset = 0;
	};

	class RendererPrototype
	{
		protected:
			cp::VulkanContext* context = nullptr;
			vk::RenderPass mainRenderPass = VK_NULL_HANDLE;
			std::unordered_map<std::string, Renderpass> renderPasses;

			virtual void CreateFixedPipelines();
			virtual void CreateMainRenderPass() = 0;
			virtual void CreateRenderPasses();
			virtual uint32_t PrepareFrame(cp::Swapchain* _swapchain);
			virtual void SubmitFrame(cp::Swapchain* _swapchain);
			virtual void PresentFrame(cp::Swapchain* _swapchain, uint32_t _index);
			virtual void EndFrame(cp::Swapchain* _swapchain);

			virtual void RenderFrame(const std::vector<InstanceGroup>& _instanceGroups) = 0;

		public:
			RendererPrototype(cp::VulkanContext* _context);
			virtual ~RendererPrototype();


			virtual void Render(const std::vector<InstanceGroup>& _instanceGroups) = 0;

			inline std::unordered_map<std::string, Renderpass>& GetRenderPasses() { return renderPasses; }
			inline constexpr cp::VulkanContext* GetContext() { return context; }

			Renderpass& RegisterRenderPass(const std::string& _name, vk::RenderPass _rp);
			Renderpass& GetRenderPass(const std::string& _name);
			std::vector<std::string> GetRenderPassNames();
	};
}
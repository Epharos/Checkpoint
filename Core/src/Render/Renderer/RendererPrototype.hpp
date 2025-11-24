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
	class RendererInstance;

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

			virtual void CreateFixedPipelines(RendererInstance& _instance);
			virtual void CreateMainRenderPass(RendererInstance& _instance) = 0;
			virtual void CreateRenderPasses(RendererInstance& _instance);
			virtual uint32_t PrepareFrame(cp::Swapchain* _swapchain);
			virtual void SubmitFrame(cp::Swapchain* _swapchain);
			virtual void PresentFrame(cp::Swapchain* _swapchain, uint32_t _index);
			virtual void EndFrame(cp::Swapchain* _swapchain);

			virtual void RenderFrame(const RendererInstance& _instance, const std::vector<InstanceGroup>& _instanceGroups) = 0;

		public:
			RendererPrototype(cp::VulkanContext* _context);
			virtual ~RendererPrototype();

			virtual void BuildForInstance(RendererInstance& _instance);

			virtual void Render(RendererInstance& _instance, const std::vector<InstanceGroup>& _instanceGroups) = 0;

			inline constexpr cp::VulkanContext* GetContext() { return context; }

	};
}
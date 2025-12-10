#pragma once

#include "../pch.hpp"
#include "../CheckpointEditor.hpp"

namespace cp 
{
	class EditorRenderer : public cp::RendererPrototype
	{
		public:
			EditorRenderer(cp::VulkanContext* _context);

			void Render(RendererInstance* _instance, const std::vector<InstanceGroup>& _instanceGroups) override;

		protected:
			cp::Camera* activeCamera = nullptr;

			constexpr static uint32_t MAX_RENDERABLE_ENTITIES = 10000;
			cp::Buffer instancedBuffer;

			void CreateMainRenderPass(RendererInstance& _instance) override;
			void RenderFrame(RendererInstance* _instance, const std::vector<InstanceGroup>& _instanceGroups) override;
			void UpdateCameraBuffer();
	};
}
#pragma once

#include "../pch.hpp"
#include "../CheckpointEditor.hpp"

namespace cp 
{
	class EditorRenderer : public cp::RendererPrototype
	{
		public:
			EditorRenderer(cp::VulkanContext* _context);
			virtual ~EditorRenderer();

		protected:
			virtual void CreateMainRenderPass() override;
			virtual void RenderFrame(const std::vector<InstanceGroup>& _instanceGroups) override;
	};
}
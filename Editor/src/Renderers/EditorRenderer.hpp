#pragma once

#include "../pch.hpp"
#include "../CheckpointEditor.hpp"

namespace cp 
{
	class EditorRenderer : public cp::Renderer 
	{
		public:
			EditorRenderer(cp::VulkanContext* _context);
			virtual void Cleanup() override;

		protected:
			virtual void SetupPipelines() override;
			virtual void CreateMainRenderPass() override;
			virtual void RenderFrame(const std::vector<InstanceGroup>& _instanceGroups) override;
	};
}
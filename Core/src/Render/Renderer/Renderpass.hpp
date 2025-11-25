#pragma once

#include "../../pch.hpp"

#include "../../Context/VulkanContext.hpp"

namespace cp
{
	class Subpass
	{
	protected:
		bool depthOnly = false;

		vk::SubpassDescription subpassDescription;

	public:
		Subpass(vk::PipelineBindPoint _bindPoint, std::vector<vk::AttachmentReference> _colorAttachments, vk::AttachmentReference _depthAttachment, bool _depthOnly = false);

		inline vk::SubpassDescription GetDescription() { return subpassDescription; }
		inline constexpr bool IsDepthOnly() { return depthOnly; }
	};

	class RenderpassDescription
	{
	protected:
		bool depthOnly = false;

		std::vector<Subpass> subpasses;
		std::vector<vk::AttachmentDescription> attachments;
		std::vector<vk::SubpassDependency> dependencies;

		std::optional<cp::PipelineData*> defaultPipeline{ std::nullopt };

		cp::VulkanContext* context;

		std::string name;

	public:
		vk::RenderPass Build();

		inline void AddSubpass(const Subpass& _subpass) { subpasses.push_back(_subpass); }
		inline void AddAttachment(const vk::AttachmentDescription& _attachment) { attachments.push_back(_attachment); }
		inline void AddDependency(const vk::SubpassDependency& _dependency) { dependencies.push_back(_dependency); }

		inline void SetDefaultPipeline(cp::PipelineData& _pipeline) { defaultPipeline = &_pipeline; }
		inline void SetDepthOnly(bool _depthOnly) { depthOnly = _depthOnly; }

		RenderpassDescription(cp::VulkanContext* _context, const std::string& _name);

		inline const std::vector<Subpass>& GetSubpasses() const { return subpasses; }
		inline const std::string& GetName() const { return name; }
		inline const std::optional<cp::PipelineData*>& GetDefaultPipeline() const { return defaultPipeline; }

		inline constexpr bool IsDepthOnly() { return depthOnly; }
	};

	class Renderpass 
	{
	protected:
		cp::VulkanContext* context;
		RenderpassDescription description;
		vk::RenderPass renderPass;

	public:
		Renderpass(cp::VulkanContext* _context, const RenderpassDescription& _description);
		~Renderpass();
	};
}
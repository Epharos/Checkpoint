#include "pch.hpp"
#include "Renderpass.hpp"

cp::Subpass::Subpass(vk::PipelineBindPoint _bindPoint, std::vector<vk::AttachmentReference> _colorAttachments, vk::AttachmentReference _depthAttachment, bool _depthOnly)
{
	depthOnly = _depthOnly;

	subpassDescription.pipelineBindPoint = _bindPoint;
	subpassDescription.colorAttachmentCount = static_cast<uint32_t>(_colorAttachments.size());
	subpassDescription.pColorAttachments = _colorAttachments.data();
	subpassDescription.pDepthStencilAttachment = &_depthAttachment;
}

vk::RenderPass cp::RenderpassDescription::Build()
{
	std::vector<vk::SubpassDescription> subpasses;

	for (auto& sp : this->subpasses)
	{
		subpasses.push_back(sp.GetDescription());
	}

	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();
	
	return context->GetDevice().createRenderPass(renderPassInfo);
}

cp::RenderpassDescription::RenderpassDescription(cp::VulkanContext* _context, const std::string& _name)
{
	context = _context;
	name = _name;
}

cp::Renderpass::Renderpass(cp::VulkanContext* _context, const RenderpassDescription& _description) : context(_context), description(_description)
{
	renderPass = description.Build();
}

cp::Renderpass::~Renderpass()
{
	context->GetDevice().destroyRenderPass(renderPass);
}

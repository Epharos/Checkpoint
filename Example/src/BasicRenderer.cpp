#include "pch.hpp"
#include "BasicRenderer.hpp"

BasicRenderer::~BasicRenderer()
{

}

void BasicRenderer::RenderFrame()
{

}

void BasicRenderer::CreateMainRenderPass()
{
	vk::AttachmentDescription depthAttachment = {};
	depthAttachment.format = Helper::Format::FindDepthFormat(context->GetPhysicalDevice());
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchain->GetFormat();
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 1;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription zPrepass = {};
	zPrepass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	zPrepass.colorAttachmentCount = 0;
	zPrepass.pDepthStencilAttachment = &depthAttachmentRef;

	vk::SubpassDescription colorizeSubpass = {};
	colorizeSubpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	colorizeSubpass.colorAttachmentCount = 1;
	colorizeSubpass.pColorAttachments = &colorAttachmentRef;
	colorizeSubpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::vector<vk::AttachmentDescription> attachments = { depthAttachment, colorAttachment };
	std::vector<vk::SubpassDescription> subpasses = { zPrepass, colorizeSubpass };

	vk::SubpassDependency dependency = {};
	dependency.srcSubpass = 0;
	dependency.dstSubpass = 1;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	dependency.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
	dependency.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eShaderRead;
	dependency.dependencyFlags = vk::DependencyFlagBits::eByRegion;

	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	mainRenderPass = context->GetDevice().createRenderPass(renderPassInfo);
}

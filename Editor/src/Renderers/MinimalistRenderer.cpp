#include "MinimalistRenderer.hpp"

void MinimalistRenderer::CreateMainRenderPass()
{
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
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription colorizeSubpass = {};
	colorizeSubpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	colorizeSubpass.colorAttachmentCount = 1;
	colorizeSubpass.pColorAttachments = &colorAttachmentRef;

	std::vector<vk::AttachmentDescription> attachments = { colorAttachment };
	std::vector<vk::SubpassDescription> subpasses = { colorizeSubpass };

	subpassCount = static_cast<uint32_t>(subpasses.size());

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
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = &dependency;

	mainRenderPass = context->GetDevice().createRenderPass(renderPassInfo);
}

void MinimalistRenderer::RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups)
{
	vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4> { 1.0f, 0.1f, 0.1f, 1.0f });
	vk::CommandBuffer commandBuffer = swapchain->GetCurrentFrame()->GetCommandBuffer();

	std::vector<vk::ClearValue> shadowMapClearValues = { clearColor };

	vk::RenderPassBeginInfo depthShadowMapRenderPassInfo = {};
	depthShadowMapRenderPassInfo.renderPass = mainRenderPass;
	depthShadowMapRenderPassInfo.framebuffer = swapchain->GetCurrentFrame()->GetRenderTarget(0)->GetFramebuffer();
	depthShadowMapRenderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	depthShadowMapRenderPassInfo.renderArea.extent = swapchain->GetExtent();
	depthShadowMapRenderPassInfo.clearValueCount = 1;
	depthShadowMapRenderPassInfo.pClearValues = shadowMapClearValues.data();

	commandBuffer.beginRenderPass(depthShadowMapRenderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.endRenderPass();
}

void MinimalistRenderer::SetupPipelines()
{

}

MinimalistRenderer::MinimalistRenderer(Context::VulkanContext* _context) : Render::Renderer(_context)
{
	
}

void MinimalistRenderer::Cleanup()
{
	context->GetDevice().waitIdle();
	context->GetDevice().destroyRenderPass(mainRenderPass);
	delete swapchain;
}

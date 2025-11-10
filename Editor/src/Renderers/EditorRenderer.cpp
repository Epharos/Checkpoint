#include "pch.hpp"
#include "EditorRenderer.hpp"

cp::EditorRenderer::EditorRenderer(cp::VulkanContext * _context) : cp::Renderer(_context)
{
	LOG_DEBUG("Constructing EditorRenderer");
}

void cp::EditorRenderer::Cleanup()
{
	context->GetDevice().waitIdle();
	context->GetDevice().destroyRenderPass(mainRenderPass);
	delete swapchain;
}

void cp::EditorRenderer::SetupPipelines()
{

}

void cp::EditorRenderer::CreateMainRenderPass()
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

	vk::SubpassDescription colorizeSubpass = {};
	colorizeSubpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	colorizeSubpass.colorAttachmentCount = 1;
	colorizeSubpass.pColorAttachments = &colorAttachmentRef;
	colorizeSubpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::vector<vk::AttachmentDescription> attachments = { depthAttachment, colorAttachment };
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

void cp::EditorRenderer::RenderFrame(const std::vector<cp::InstanceGroup>& _instanceGroups)
{
	vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4> { 0.2f, 0.2f, 0.2f, 1.0f });
	vk::ClearDepthStencilValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::CommandBuffer commandBuffer = swapchain->GetCurrentFrame()->GetCommandBuffer();

	std::vector<vk::ClearValue> rpClearValues = { clearDepth, clearColor };

	vk::Viewport vp = vk::Viewport(0, 0, swapchain->GetExtent().width, swapchain->GetExtent().height, 0, 1);
	vk::Rect2D scissor = vk::Rect2D(vk::Offset2D(0, 0), swapchain->GetExtent());

	commandBuffer.setViewport(0, vp);
	commandBuffer.setScissor(0, scissor);

	vk::RenderPassBeginInfo rpInfo = {};
	rpInfo.renderPass = mainRenderPass;
	rpInfo.framebuffer = swapchain->GetCurrentFrame()->GetMainRenderTarget()->GetFramebuffer();
	rpInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	rpInfo.renderArea.extent = swapchain->GetExtent();
	rpInfo.clearValueCount = static_cast<uint32_t>(rpClearValues.size());
	rpInfo.pClearValues = rpClearValues.data();

	commandBuffer.beginRenderPass(rpInfo, vk::SubpassContents::eInline);

	cp::Mesh* currentMesh = nullptr;
	cp::Material* currentMaterial = nullptr;
	cp::MaterialInstance* currentMaterialInstance = nullptr;

	/*for (const auto& instanceGroup : _instanceGroups)
	{
		vk::DeviceSize offset(0);

		if (currentMaterial != instanceGroup.material)
		{
			currentMaterial = instanceGroup.material;
			currentMaterial->BindMaterial(commandBuffer);
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentMaterial->GetPipelineLayout(), 0, context->GetDescriptorSetManager()->GetDescriptorSet("Render Camera"), nullptr);
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentMaterial->GetPipelineLayout(), 1, context->GetDescriptorSetManager()->GetDescriptorSet("Instance Model"), nullptr);
		}

		if (currentMaterialInstance != instanceGroup.materialInstance)
		{
			currentMaterialInstance = instanceGroup.materialInstance;
			currentMaterialInstance->BindMaterialInstance(commandBuffer);
		}

		if (currentMesh != instanceGroup.mesh)
		{
			currentMesh = instanceGroup.mesh;
			commandBuffer.bindVertexBuffers(0, 1, &currentMesh->GetVertexBuffer(), &offset);
			commandBuffer.bindIndexBuffer(currentMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);
		}

		commandBuffer.drawIndexed(instanceGroup.mesh->GetIndexCount(), instanceGroup.transforms.size(), 0, 0, instanceGroup.instanceOffset);
	}*/

	commandBuffer.endRenderPass();
}
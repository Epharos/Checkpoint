#include "pch.hpp"
#include "EditorRenderer.hpp"

cp::EditorRenderer::EditorRenderer(cp::VulkanContext * _context) : cp::RendererPrototype(_context)
{
	LOG_DEBUG("Constructing EditorRenderer");
}

void cp::EditorRenderer::Render(RendererInstance* _instance, const std::vector<InstanceGroup>& _instanceGroups)
{
	uint32 index = PrepareFrame(_instance->GetSwapchain());

	RenderFrame(_instance, _instanceGroups);

	SubmitFrame(_instance->GetSwapchain());
	PresentFrame(_instance->GetSwapchain(), index);

	EndFrame(_instance->GetSwapchain());
}

void cp::EditorRenderer::CreateMainRenderPass(RendererInstance& _instance)
{
	LOG_DEBUG("No more renderpass creation needed");
}

void cp::EditorRenderer::RenderFrame(RendererInstance* _instance, const std::vector<InstanceGroup>& _instanceGroups)
{
	vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4> { 0.8f, 0.2f, 0.2f, 1.0f });
	vk::ClearDepthStencilValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::CommandBuffer commandBuffer = _instance->GetSwapchain()->GetCurrentFrame()->GetCommandBuffer();

	vk::RenderingAttachmentInfoKHR colorAttachmentInfo;
	colorAttachmentInfo.imageView = _instance->GetSwapchain()->GetCurrentFrame()->GetMainRenderTarget()->GetAttachment(1)->GetImageView();
	colorAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimalKHR;
	colorAttachmentInfo.clearValue = clearColor;
	colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;

	vk::RenderingAttachmentInfoKHR depthAttachmentInfo;
	depthAttachmentInfo.imageView = _instance->GetSwapchain()->GetCurrentFrame()->GetMainRenderTarget()->GetAttachment(0)->GetImageView();
	depthAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimalKHR;
	depthAttachmentInfo.clearValue = clearDepth;
	depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;

	vk::RenderingInfo renderingInfo;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachmentInfo;
	renderingInfo.pDepthAttachment = &depthAttachmentInfo;
	//renderingInfo.pStencilAttachment = &depthAttachmentInfo;
	renderingInfo.layerCount = 1;
	renderingInfo.renderArea = vk::Rect2D{ vk::Offset2D{}, _instance->GetSwapchain()->GetExtent() };

	commandBuffer.beginRenderingKHR(renderingInfo, context->GetDynamicLoader());

	commandBuffer.endRenderingKHR(context->GetDynamicLoader());
}

//void cp::EditorRenderer::RenderFrame(const std::vector<cp::InstanceGroup>& _instanceGroups)
//{
//	vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4> { 0.2f, 0.2f, 0.2f, 1.0f });
//	vk::ClearDepthStencilValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
//
//	vk::CommandBuffer commandBuffer = swapchain->GetCurrentFrame()->GetCommandBuffer();
//
//	std::vector<vk::ClearValue> rpClearValues = { clearDepth, clearColor };
//
//	vk::Viewport vp = vk::Viewport(0, 0, swapchain->GetExtent().width, swapchain->GetExtent().height, 0, 1);
//	vk::Rect2D scissor = vk::Rect2D(vk::Offset2D(0, 0), swapchain->GetExtent());
//
//	commandBuffer.setViewport(0, vp);
//	commandBuffer.setScissor(0, scissor);
//
//	std::string currentPassName = "Main";
//	vk::RenderPass currentPass = renderPasses.at(currentPassName).GetRenderPass();
//
//	vk::RenderPassBeginInfo rpInfo = {};
//	rpInfo.renderPass = currentPass;
//	rpInfo.framebuffer = swapchain->GetCurrentFrame()->GetMainRenderTarget()->GetFramebuffer();
//	rpInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
//	rpInfo.renderArea.extent = swapchain->GetExtent();
//	rpInfo.clearValueCount = static_cast<uint32_t>(rpClearValues.size());
//	rpInfo.pClearValues = rpClearValues.data();
//
//	commandBuffer.beginRenderPass(rpInfo, vk::SubpassContents::eInline);
//
//	cp::Mesh* currentMesh = nullptr;
//	cp::Material* currentMaterial = nullptr;
//	cp::MaterialInstance* currentMaterialInstance = nullptr;
//
//	/*for (const auto& instanceGroup : _instanceGroups)
//	{
//		vk::DeviceSize offset(0);
//
//		if (currentMaterial != instanceGroup.material)
//		{
//			currentMaterial = instanceGroup.material;
//			currentMaterial->BindMaterial(commandBuffer);
//			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentMaterial->GetPipelineLayout(), 0, context->GetDescriptorSetManager()->GetDescriptorSet("Render Camera"), nullptr);
//			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentMaterial->GetPipelineLayout(), 1, context->GetDescriptorSetManager()->GetDescriptorSet("Instance Model"), nullptr);
//		}
//
//		if (currentMaterialInstance != instanceGroup.materialInstance)
//		{
//			currentMaterialInstance = instanceGroup.materialInstance;
//			currentMaterialInstance->BindMaterialInstance(commandBuffer);
//		}
//
//		if (currentMesh != instanceGroup.mesh)
//		{
//			currentMesh = instanceGroup.mesh;
//			commandBuffer.bindVertexBuffers(0, 1, &currentMesh->GetVertexBuffer(), &offset);
//			commandBuffer.bindIndexBuffer(currentMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);
//		}
//
//		commandBuffer.drawIndexed(instanceGroup.mesh->GetIndexCount(), instanceGroup.transforms.size(), 0, 0, instanceGroup.instanceOffset);
//	}*/
//
//	commandBuffer.endRenderPass();
//}
#include "pch.hpp"
#include "EditorRenderer.hpp"

cp::EditorRenderer::EditorRenderer(cp::VulkanContext * _context) : cp::RendererPrototype(_context)
{
	LOG_DEBUG("Constructing EditorRenderer");

	cp::RenderpassDescription colorRenderpassDesc(_context, "Color Pass");
	renderPassDescriptions.emplace("Color Pass", colorRenderpassDesc);
}

void cp::EditorRenderer::Render(RendererInstance* _instance, const std::vector<InstanceGroup>& _instanceGroups)
{
	if (!postProcessingSetup)
	{
		SetupPostProcessingPass(_instance);
		postProcessingSetup = true;
	}

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
	vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4> { 0.8f, 0.8f, 0.8f, 1.0f });
	vk::ClearDepthStencilValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::DeviceSize offset(0);

	vk::CommandBuffer commandBuffer = _instance->GetSwapchain()->GetCurrentFrame()->GetCommandBuffer();

	std::shared_ptr<cp::RenderTargetAttachment> hdrAttachment = _instance->GetSwapchain()->GetCurrentFrame()->GetMainRenderTarget()->GetAttachment(2);

	vk::ImageMemoryBarrier2 imageBarrier2{};
	imageBarrier2.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imageBarrier2.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
	imageBarrier2.srcAccessMask = vk::AccessFlagBits2::eNone;
	imageBarrier2.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
	imageBarrier2.srcStageMask = vk::PipelineStageFlagBits2::eNone;
	imageBarrier2.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	imageBarrier2.image = hdrAttachment->GetImage();
	imageBarrier2.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageBarrier2.subresourceRange.baseMipLevel = 0;
	imageBarrier2.subresourceRange.levelCount = 1;
	imageBarrier2.subresourceRange.baseArrayLayer = 0;
	imageBarrier2.subresourceRange.layerCount = 1;

	vk::DependencyInfo dependencyInfo{};
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &imageBarrier2;

	commandBuffer.pipelineBarrier2(dependencyInfo);


	vk::RenderingAttachmentInfoKHR hdrColorAttachmentInfo;
	hdrColorAttachmentInfo.imageView = hdrAttachment->GetImageView();
	hdrColorAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimalKHR;
	hdrColorAttachmentInfo.clearValue = clearColor;
	hdrColorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	hdrColorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;

	vk::RenderingAttachmentInfoKHR depthAttachmentInfo;
	depthAttachmentInfo.imageView = _instance->GetSwapchain()->GetCurrentFrame()->GetMainRenderTarget()->GetAttachment(0)->GetImageView();
	depthAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimalKHR;
	depthAttachmentInfo.clearValue = clearDepth;
	depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;

	vk::RenderingInfo renderingInfo;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &hdrColorAttachmentInfo;
	renderingInfo.pDepthAttachment = &depthAttachmentInfo;
	//renderingInfo.pStencilAttachment = &depthAttachmentInfo;
	renderingInfo.layerCount = 1;
	renderingInfo.renderArea = vk::Rect2D{ vk::Offset2D{}, _instance->GetSwapchain()->GetExtent() };

	vk::Viewport vp = vk::Viewport(0, 0, _instance->GetSwapchain()->GetExtent().width, _instance->GetSwapchain()->GetExtent().height, 0, 1);
	vk::Rect2D scissor = vk::Rect2D(vk::Offset2D(0, 0), _instance->GetSwapchain()->GetExtent());

	commandBuffer.beginRenderingKHR(renderingInfo, context->GetDynamicLoader());

	commandBuffer.setViewport(0, vp);
	commandBuffer.setScissor(0, scissor);

	cp::Mesh* currentMesh = nullptr;
	cp::Material* currentMaterial = nullptr;
	cp::MaterialInstance* currentMaterialInstance = nullptr;

	for (const auto& instanceGroup : _instanceGroups)
	{
		if (currentMaterial != instanceGroup.material)
		{
			currentMaterial = instanceGroup.material;
			currentMaterial->BindMaterial(commandBuffer, "Color Pass");
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentMaterial->GetPipelineLayout("Color Pass"), 0, context->GetDescriptorSetManager()->GetDescriptorSet("Global Unlit"), nullptr);
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentMaterial->GetPipelineLayout("Color Pass"), 1, context->GetDescriptorSetManager()->GetDescriptorSet("Instanced Drawing"), nullptr);
		}

		if (currentMaterialInstance != instanceGroup.materialInstance)
		{
			currentMaterialInstance = instanceGroup.materialInstance;
			currentMaterialInstance->BindMaterialInstance(commandBuffer, "Color Pass");
		}

		if (currentMesh != instanceGroup.mesh)
		{
			currentMesh = instanceGroup.mesh;
			commandBuffer.bindVertexBuffers(0, 1, &currentMesh->GetVertexBuffer(), &offset);
			commandBuffer.bindIndexBuffer(currentMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);
		}

		Helper::Memory::MapMemory(
			context->GetDevice(),
			_instance->GetInstancedBuffer().memory,
			sizeof(cp::TransformData) * instanceGroup.transforms.size(),
			sizeof(cp::TransformData) * instanceGroup.instanceOffset,
			instanceGroup.transforms.data()
		);

		commandBuffer.drawIndexed(currentMesh->GetIndexCount(), instanceGroup.transforms.size(), 0, 0, instanceGroup.instanceOffset);
	}

	commandBuffer.endRenderingKHR(context->GetDynamicLoader());

	// --- Post-Processing Pass (Tonemapping)

	vk::ImageMemoryBarrier2 imageBarrierHDR{};
	imageBarrierHDR.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
	imageBarrierHDR.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imageBarrierHDR.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
	imageBarrierHDR.dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead;
	imageBarrierHDR.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	imageBarrierHDR.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
	imageBarrierHDR.image = hdrAttachment->GetImage();
	imageBarrierHDR.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageBarrierHDR.subresourceRange.baseMipLevel = 0;
	imageBarrierHDR.subresourceRange.levelCount = 1;
	imageBarrierHDR.subresourceRange.baseArrayLayer = 0;
	imageBarrierHDR.subresourceRange.layerCount = 1;

	vk::ImageMemoryBarrier2 imageBarrierSwapchain{};
	imageBarrierSwapchain.oldLayout = vk::ImageLayout::ePresentSrcKHR;
	imageBarrierSwapchain.newLayout = vk::ImageLayout::eAttachmentOptimalKHR;
	imageBarrierSwapchain.srcAccessMask = vk::AccessFlagBits2::eNone;
	imageBarrierSwapchain.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
	imageBarrierSwapchain.srcStageMask = vk::PipelineStageFlagBits2::eNone;
	imageBarrierSwapchain.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	imageBarrierSwapchain.image = _instance->GetSwapchain()->GetCurrentFrame()->GetMainRenderTarget()->GetAttachment(1)->GetImage();
	imageBarrierSwapchain.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageBarrierSwapchain.subresourceRange.baseMipLevel = 0;
	imageBarrierSwapchain.subresourceRange.levelCount = 1;
	imageBarrierSwapchain.subresourceRange.baseArrayLayer = 0;
	imageBarrierSwapchain.subresourceRange.layerCount = 1;

	vk::ImageMemoryBarrier2 imageBarriers[] = { imageBarrierHDR, imageBarrierSwapchain };

	dependencyInfo.imageMemoryBarrierCount = 2;
	dependencyInfo.pImageMemoryBarriers = imageBarriers;

	commandBuffer.pipelineBarrier2(dependencyInfo);

	cp::DescriptorSetUpdate update;
	update.updateType = cp::DescriptorSetUpdateType::IMAGE;
	update.imageView = hdrAttachment->GetImageView();
	update.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	update.sampler = hdrAttachment->GetSampler();
	update.dstBinding = 0;
	update.dstArrayElement = 0;
	update.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	update.descriptorCount = 1;

	context->GetDescriptorSetManager()->UpdateDescriptorSet("Tonemapping_POSTFX_2", update);

	vk::RenderingAttachmentInfoKHR swapchainColorAttachmentInfo;
	swapchainColorAttachmentInfo.imageView = _instance->GetSwapchain()->GetCurrentFrame()->GetMainRenderTarget()->GetAttachment(1)->GetImageView();
	swapchainColorAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimalKHR;
	swapchainColorAttachmentInfo.clearValue = clearColor;
	swapchainColorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	swapchainColorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;

	vk::RenderingInfo postProcessRenderingInfo;
	postProcessRenderingInfo.colorAttachmentCount = 1;
	postProcessRenderingInfo.pColorAttachments = &swapchainColorAttachmentInfo;
	postProcessRenderingInfo.layerCount = 1;
	postProcessRenderingInfo.renderArea = vk::Rect2D{ vk::Offset2D{}, _instance->GetSwapchain()->GetExtent() };

	commandBuffer.beginRenderingKHR(postProcessRenderingInfo, context->GetDynamicLoader());
	commandBuffer.setViewport(0, vp);
	commandBuffer.setScissor(0, scissor);

	postProcessMaterialInstance->GetMaterial()->BindMaterial(commandBuffer, "Color Pass");
	postProcessMaterialInstance->BindMaterialInstance(commandBuffer, "Color Pass");
	commandBuffer.bindVertexBuffers(0, 1, &quadMesh->GetVertexBuffer(), &offset);
	commandBuffer.bindIndexBuffer(quadMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);

	commandBuffer.drawIndexed(quadMesh->GetIndexCount(), 1, 0, 0, 0);
	commandBuffer.endRenderingKHR(context->GetDynamicLoader());
}

void cp::EditorRenderer::SetupPostProcessingPass(RendererInstance* _instance)
{
	postProcessMaterialInstance = cp::ResourceManager::Get()->GetOrLoad<cp::MaterialInstance>("Tonemapping POSTFX", cp::CheckpointEditor::CurrentProject.GetResourcePath() + "/Materials/Tonemapping.matinstance");
	postProcessMaterialInstance->GetMaterial()->Reload(*this, _instance->GetSwapchain()->GetFormat());
	postProcessMaterialInstance->ValidateData();
	postProcessMaterialInstance->UpdateDescriptorSets();

	quadMesh = cp::ResourceManager::Get()->GetOrLoad<cp::Mesh>("Fullscreen Quad", cp::CheckpointEditor::CurrentProject.GetResourcePath() + "/Models/quad.obj");
}

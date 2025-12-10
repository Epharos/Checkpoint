#include "pch.hpp"
#include "EditorRenderer.hpp"

cp::EditorRenderer::EditorRenderer(cp::VulkanContext * _context) : cp::RendererPrototype(_context)
{
	LOG_DEBUG("Constructing EditorRenderer");

	cp::RenderpassDescription colorRenderpassDesc(_context, "Color Pass");
	renderPassDescriptions.emplace("Color Pass", colorRenderpassDesc);

	//cp::RenderpassDescription zPrepassDesc(_context, "Z Pre-Pass");
	//zPrepassDesc.SetDepthOnly(true);
	//renderPassDescriptions.emplace("Z Pre-Pass", zPrepassDesc);

	//cp::RenderpassDescription gBufferPassDesc(_context, "G-Buffer Pass");
	//renderPassDescriptions.emplace("G-Buffer Pass", gBufferPassDesc);

	instancedBuffer = Helper::Memory::CreateBuffer(
		_context->GetDevice(),
		_context->GetPhysicalDevice(),
		sizeof(cp::TransformData) * MAX_RENDERABLE_ENTITIES,
		vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	cp::DescriptorSetUpdate instanceBufferUpdate;
	instanceBufferUpdate.updateType = cp::DescriptorSetUpdateType::BUFFER;
	instanceBufferUpdate.buffer = instancedBuffer.buffer;
	instanceBufferUpdate.offset = 0;
	instanceBufferUpdate.range = sizeof(cp::TransformData) * MAX_RENDERABLE_ENTITIES;
	instanceBufferUpdate.dstBinding = 0;
	instanceBufferUpdate.dstArrayElement = 0;
	instanceBufferUpdate.descriptorType = vk::DescriptorType::eStorageBuffer;
	instanceBufferUpdate.descriptorCount = 1;
	context->GetDescriptorSetManager()->UpdateDescriptorSet("Instanced Drawing", { instanceBufferUpdate });
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
	if (!activeCamera)
	{
		//LOG_WARNING("No active camera set for EditorRenderer, cannot render frame");
		return;
	}

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

	cp::Mesh* currentMesh = nullptr;
	cp::Material* currentMaterial = nullptr;
	cp::MaterialInstance* currentMaterialInstance = nullptr;

	return; //tmp

	for (const auto& instanceGroup : _instanceGroups)
	{
		vk::DeviceSize offset(0);

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
			//currentMaterialInstance->BindMaterialInstance(commandBuffer);
		}

		if (currentMesh != instanceGroup.mesh)
		{
			currentMesh = instanceGroup.mesh;
			commandBuffer.bindVertexBuffers(0, 1, &currentMesh->GetVertexBuffer(), &offset);
			commandBuffer.bindIndexBuffer(currentMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);
		}

		Helper::Memory::MapMemory(
			context->GetDevice(),
			instancedBuffer.memory,
			sizeof(cp::TransformData) * instanceGroup.transforms.size(),
			sizeof(cp::TransformData) * instanceGroup.instanceOffset,
			instanceGroup.transforms.data()
		);

		commandBuffer.drawIndexed(instanceGroup.mesh->GetIndexCount(), instanceGroup.transforms.size(), 0, 0, instanceGroup.instanceOffset);
	}

	commandBuffer.endRenderingKHR(context->GetDynamicLoader());
}

void cp::EditorRenderer::UpdateCameraBuffer()
{
	if(!activeCamera)
	{
		LOG_WARNING("No active camera set for EditorRenderer, cannot update camera buffer");
		return;
	}

	cp::DescriptorSetUpdate cameraSetUpdate;
	cameraSetUpdate.updateType = cp::DescriptorSetUpdateType::BUFFER;
	cameraSetUpdate.buffer = activeCamera->GetUBOBuffer();
	cameraSetUpdate.offset = 0;
	cameraSetUpdate.range = sizeof(cp::CameraUBO);
	cameraSetUpdate.dstBinding = 0;
	cameraSetUpdate.dstArrayElement = 0;
	cameraSetUpdate.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraSetUpdate.descriptorCount = 1;

	context->GetDescriptorSetManager()->UpdateDescriptorSet("Global Unlit", { cameraSetUpdate });
}
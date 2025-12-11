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

	vk::CommandBuffer commandBuffer = _instance->GetSwapchain()->GetCurrentFrame()->GetCommandBuffer();

	vk::RenderingAttachmentInfoKHR hdrColorAttachmentInfo;
	hdrColorAttachmentInfo.imageView = _instance->GetSwapchain()->GetCurrentFrame()->GetMainRenderTarget()->GetAttachment(2)->GetImageView();
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

		commandBuffer.drawIndexed(instanceGroup.mesh->GetIndexCount(), instanceGroup.transforms.size(), 0, 0, instanceGroup.instanceOffset);
	}

	commandBuffer.endRenderingKHR(context->GetDynamicLoader());
}
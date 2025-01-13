#include "pch.hpp"
#include "BasicRenderer.hpp"

BasicRenderer::~BasicRenderer()
{

}

void BasicRenderer::Cleanup()
{
	Renderer::Cleanup();
	Helper::Memory::DestroyBuffer(context->GetDevice(), instancedBuffer, instancedBufferMemory);
}

void BasicRenderer::RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups)
{
	vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.0f});
	vk::ClearDepthStencilValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	std::vector<vk::ClearValue> clearValues = { clearDepth, clearColor };

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = mainRenderPass;
	renderPassInfo.framebuffer = swapchain->GetCurrentFrame()->GetMainRenderTarget()->GetFramebuffer();
	renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	renderPassInfo.renderArea.extent = swapchain->GetExtent();
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vk::CommandBuffer commandBuffer = swapchain->GetCurrentFrame()->GetCommandBuffer();
	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	// RENDER

	commandBuffer.nextSubpass(vk::SubpassContents::eInline);

	Pipeline::PipelineData boundPipeline = pipelinesManager->GetPipeline({ "Basic" });
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, boundPipeline.pipeline);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, boundPipeline.pipelineLayout, 0, 1, &descriptorSetManager->GetDescriptorSet("Camera"), 0, nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, boundPipeline.pipelineLayout, 1, 1, &descriptorSetManager->GetDescriptorSet("Instance Model"), 0, nullptr);

	for (auto& instanceGroup : _instanceGroups)
	{
		Helper::Memory::MapMemory(context->GetDevice(), instancedBufferMemory, sizeof(glm::mat4) * instanceGroup.transforms.size(), (void*)instanceGroup.transforms.data());

		vk::DeviceSize offset[1] = {0};

		commandBuffer.bindVertexBuffers(0, 1, &instanceGroup.mesh->GetVertexBuffer(), offset);
		commandBuffer.bindIndexBuffer(instanceGroup.mesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);

		commandBuffer.drawIndexed(instanceGroup.mesh->GetIndexCount(), instanceGroup.transforms.size(), 0, 0, 0);
	}

	

	commandBuffer.endRenderPass();
}

void BasicRenderer::SetupPipelines()
{
	instancedBuffer = Helper::Memory::CreateBuffer(context->GetDevice(), context->GetPhysicalDevice(), sizeof(glm::mat4) * 1000, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, instancedBufferMemory);

	vk::DescriptorSetLayout cameraLayout = descriptorSetLayoutsManager->CreateDescriptorSetLayout("Camera", 
		{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
		});

	vk::DescriptorSetLayout instancedModelLayout = descriptorSetLayoutsManager->CreateDescriptorSetLayout("Instanced Model",
		{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex)
		});

	descriptorSetManager->CreateDescriptorSets({ "Camera", "Instance Model" }, { cameraLayout, instancedModelLayout });

	if (mainCamera)
		descriptorSetManager->UpdateDescriptorSet("Camera", { mainCamera->GetUBOBuffer(), 0, sizeof(Render::CameraUBO), 0, 0, vk::DescriptorType::eUniformBuffer, 1});

	descriptorSetManager->UpdateDescriptorSet("Instance Model", { instancedBuffer, 0, sizeof(glm::mat4) * 1000, 0, 0, vk::DescriptorType::eStorageBuffer, 1 });

	const std::vector<vk::DescriptorSetLayout> layouts = { cameraLayout, instancedModelLayout };

	vk::PipelineLayout layout = layoutsManager->GetOrCreateLayout(layouts, {});

	Pipeline::PipelineCreateData pipelineData = {};
	pipelineData.config.name = "Basic";

	pipelineData.descriptorSetLayouts = layouts;

	vk::Viewport* vp = new vk::Viewport;
	vp->x = 0.f;
	vp->y = 0.f;
	vp->width = swapchain->GetExtent().width;
	vp->height = swapchain->GetExtent().height;
	vp->minDepth = 0.f;
	vp->maxDepth = 1.f;

	vk::Rect2D* scisor = new vk::Rect2D;
	scisor->extent = swapchain->GetExtent();
	scisor->offset = vk::Offset2D(0, 0);

	vk::PipelineColorBlendAttachmentState* colorBlendAttachment = new vk::PipelineColorBlendAttachmentState;
	colorBlendAttachment->colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment->blendEnable = VK_FALSE;

	vk::VertexInputBindingDescription* bindingDescription = new vk::VertexInputBindingDescription(0, sizeof(Resource::Vertex), vk::VertexInputRate::eVertex);

	vk::VertexInputAttributeDescription* attributeDescriptions = new vk::VertexInputAttributeDescription[5];
	attributeDescriptions[0] = vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, position));
	attributeDescriptions[1] = vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, normal));
	attributeDescriptions[2] = vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Resource::Vertex, uv));
	attributeDescriptions[3] = vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, tangent));
	attributeDescriptions[4] = vk::VertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, bitangent));

	pipelineData.createInfo = vk::GraphicsPipelineCreateInfo();
	pipelineData.createInfo.layout = layout;
	pipelineData.createInfo.renderPass = mainRenderPass;
	pipelineData.createInfo.subpass = 1;
	pipelineData.createInfo.pDepthStencilState = new vk::PipelineDepthStencilStateCreateInfo(vk::PipelineDepthStencilStateCreateFlags(), VK_TRUE, VK_TRUE, vk::CompareOp::eLess);
	pipelineData.createInfo.pViewportState = new vk::PipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, vp, 1, scisor);
	pipelineData.createInfo.pRasterizationState = new vk::PipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(), VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
	pipelineData.createInfo.pMultisampleState = new vk::PipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags(), vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
	pipelineData.createInfo.pColorBlendState = new vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, 1, colorBlendAttachment, { 0.f, 0.f, 0.f, 0.f });
	pipelineData.createInfo.pInputAssemblyState = new vk::PipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	pipelineData.createInfo.pVertexInputState = new vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(), 1, bindingDescription, 5, attributeDescriptions);

	pipelineData.shaderFile = "Shaders/Shader.spv";
	pipelineData.mains = { 
		{ vk::ShaderStageFlagBits::eVertex, "vertexMain" }, 
		{ vk::ShaderStageFlagBits::eFragment, "pixelMain" } 
	};

	pipelinesManager->CreatePipeline(pipelineData);

	//TODO : Introduce pipeline cache
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
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	mainRenderPass = context->GetDevice().createRenderPass(renderPassInfo);
}

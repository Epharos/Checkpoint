#include "MinimalistRenderer.hpp"

void MinimalistRenderer::CreateMainRenderPass()
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

void MinimalistRenderer::RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups)
{
	vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4> { 1.0f, 0.0f, 0.0f, 1.0f });
	vk::ClearDepthStencilValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::CommandBuffer commandBuffer = swapchain->GetCurrentFrame()->GetCommandBuffer();

	std::vector<vk::ClearValue> shadowMapClearValues = { clearDepth, clearColor };

	vk::Viewport vp = vk::Viewport(0, 0, swapchain->GetExtent().width, swapchain->GetExtent().height, 0, 1);
	vk::Rect2D scissor = vk::Rect2D(vk::Offset2D(0, 0), swapchain->GetExtent());

	vk::RenderPassBeginInfo depthShadowMapRenderPassInfo = {};
	depthShadowMapRenderPassInfo.renderPass = mainRenderPass;
	depthShadowMapRenderPassInfo.framebuffer = swapchain->GetCurrentFrame()->GetMainRenderTarget()->GetFramebuffer();
	depthShadowMapRenderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	depthShadowMapRenderPassInfo.renderArea.extent = swapchain->GetExtent();
	depthShadowMapRenderPassInfo.clearValueCount = 2;
	depthShadowMapRenderPassInfo.pClearValues = shadowMapClearValues.data();

	commandBuffer.beginRenderPass(depthShadowMapRenderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.setViewport(0, vp);
	commandBuffer.setScissor(0, scissor);

	Pipeline::PipelineCreateData config = {};
	config.config.name = "Colored";

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, context->GetPipelinesManager()->GetPipeline(config).pipeline);

	vk::Buffer vertexBuffer = quadMesh->GetVertexBuffer();
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, offsets);
	commandBuffer.bindIndexBuffer(quadMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);

	commandBuffer.drawIndexed(6, 1, 0, 0, 0);

	commandBuffer.endRenderPass();
}

void MinimalistRenderer::SetupPipelines()
{
	Pipeline::DescriptorSetLayoutsManager* descriptorSetLayoutsManager = context->GetDescriptorSetLayoutsManager();
	Pipeline::DescriptorSetManager* descriptorSetManager = context->GetDescriptorSetManager();
	Pipeline::PipelinesManager* pipelinesManager = context->GetPipelinesManager();
	Pipeline::LayoutsManager* layoutsManager = context->GetLayoutsManager();

	vk::PipelineLayout colorLayout = layoutsManager->GetOrCreateLayout({}, {});

	Pipeline::PipelineCreateData pipelineData = {};
	pipelineData.config.name = "Colored";

	std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

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
	pipelineData.createInfo.layout = colorLayout;
	pipelineData.createInfo.renderPass = mainRenderPass;
	pipelineData.createInfo.subpass = 0;
	pipelineData.createInfo.pDynamicState = new vk::PipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data());
	pipelineData.createInfo.pDepthStencilState = new vk::PipelineDepthStencilStateCreateInfo(vk::PipelineDepthStencilStateCreateFlags(), VK_TRUE, VK_TRUE, vk::CompareOp::eLess);
	pipelineData.createInfo.pViewportState = new vk::PipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);
	pipelineData.createInfo.pRasterizationState = new vk::PipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(), VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
	pipelineData.createInfo.pMultisampleState = new vk::PipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags(), vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
	pipelineData.createInfo.pColorBlendState = new vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, 1, colorBlendAttachment, { 0.f, 0.f, 0.f, 0.f });
	pipelineData.createInfo.pInputAssemblyState = new vk::PipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	pipelineData.createInfo.pVertexInputState = new vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(), 1, bindingDescription, 5, attributeDescriptions);

	pipelineData.shaderFile = "DEBUG/HALF_QUAD.spv";
	pipelineData.mains = {
		{ vk::ShaderStageFlagBits::eVertex, "vertexMain" },
		{ vk::ShaderStageFlagBits::eFragment, "pixelMain" }
	};

	pipelinesManager->CreatePipeline(pipelineData);
}

MinimalistRenderer::MinimalistRenderer(Context::VulkanContext* _context) : Render::Renderer(_context)
{
	Resource::Vertex quadVertices[] = {
		{ { 0.5f, 0.5f, 0.0f } },
		{ { -0.5f, 0.5f, 0.0f } },
		{ { -0.5f, -0.5f, 0.0f } },
		{ { 0.5f, -0.5f, 0.0f } }
	};

	uint32_t quadIndices[] = { 0, 2, 1, 2, 0, 3 };

	quadMesh = new Resource::Mesh(*context, std::vector<Resource::Vertex>(quadVertices, quadVertices + 4), std::vector<uint32_t>(quadIndices, quadIndices + 6));
}

void MinimalistRenderer::Cleanup()
{
	context->GetDevice().waitIdle();
	context->GetDevice().destroyRenderPass(mainRenderPass);
	delete swapchain;
}

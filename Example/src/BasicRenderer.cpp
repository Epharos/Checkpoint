#include "pch.hpp"
#include "BasicRenderer.hpp"



void LogMat4(glm::mat4 _mat);

BasicRenderer::BasicRenderer(Context::VulkanContext* _context, const uint32_t& _maxRenderableEntities) :
	Render::Renderer(_context), MAX_RENDERABLE_ENTITIES(_maxRenderableEntities), shadowMapRT(nullptr)
{

}

BasicRenderer::~BasicRenderer()
{

}

void BasicRenderer::Cleanup()
{
	context->GetDevice().destroyRenderPass(shadowMapRenderPass);
	delete shadowMapRT;
	Renderer::Cleanup();
	Helper::Memory::DestroyBuffer(context->GetDevice(), instancedBuffer, instancedBufferMemory);
	Helper::Memory::DestroyBuffer(context->GetDevice(), sunLightBuffer, sunLightBufferMemory);
	Helper::Memory::DestroyBuffer(context->GetDevice(), shadowMapCascadesBuffer, shadowMapCascadesBufferMemory);
}

void BasicRenderer::RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups)
{
	Helper::Memory::MapMemory(context->GetDevice(), sunLightBufferMemory, sizeof(SunLight), &sunLight);

	vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
	vk::ClearDepthStencilValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	Resource::Mesh* currentMesh = nullptr;

	vk::CommandBuffer commandBuffer = swapchain->GetCurrentFrame()->GetCommandBuffer();

#pragma region Shadow Map Render Pass
	std::vector<vk::ClearValue> shadowMapClearValues = { clearDepth };

	vk::RenderPassBeginInfo depthShadowMapRenderPassInfo = {};
	depthShadowMapRenderPassInfo.renderPass = shadowMapRenderPass;
	depthShadowMapRenderPassInfo.framebuffer = shadowMapRT->GetFramebuffer();
	depthShadowMapRenderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	depthShadowMapRenderPassInfo.renderArea.extent = vk::Extent2D(4096, 4096);
	depthShadowMapRenderPassInfo.clearValueCount = 1;
	depthShadowMapRenderPassInfo.pClearValues = shadowMapClearValues.data();

	commandBuffer.beginRenderPass(depthShadowMapRenderPassInfo, vk::SubpassContents::eInline);

	Pipeline::PipelineData pipelineData = context->GetPipelinesManager()->GetPipeline({ "Depth Shadow Map" });
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData.pipeline);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 0, context->GetDescriptorSetManager()->GetDescriptorSet("Shadow Map Camera"), nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineData.pipelineLayout, 1, context->GetDescriptorSetManager()->GetDescriptorSet("Instance Model"), nullptr);

	for (int i = 0; i < sunLight.cascadeCount; i++)
	{
		commandBuffer.pushConstants(pipelineData.pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry, 0, sizeof(uint32_t), &i);

		for (auto& instanceGroup : _instanceGroups)
		{
			vk::DeviceSize offset(0);

			if (currentMesh != instanceGroup.mesh)
			{
				currentMesh = instanceGroup.mesh;
				commandBuffer.bindVertexBuffers(0, 1, &currentMesh->GetVertexBuffer(), &offset);
				commandBuffer.bindIndexBuffer(currentMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);
			}

			Helper::Memory::MapMemory(context->GetDevice(), instancedBufferMemory, sizeof(Render::TransformData) * instanceGroup.transforms.size(), instanceGroup.instanceOffset * sizeof(Render::TransformData), instanceGroup.transforms.data());

			commandBuffer.drawIndexed(instanceGroup.mesh->GetIndexCount(), instanceGroup.transforms.size(), 0, 0, instanceGroup.instanceOffset);
		}
	}

	commandBuffer.endRenderPass();
#pragma endregion

	vk::ImageMemoryBarrier shadowMapBarrier = {};
	shadowMapBarrier.sType = vk::StructureType::eImageMemoryBarrier;
	shadowMapBarrier.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	shadowMapBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	shadowMapBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	shadowMapBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	shadowMapBarrier.image = shadowMapRT->GetAttachment(0)->GetImage();
	shadowMapBarrier.subresourceRange = { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, sunLight.cascadeCount };

	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eLateFragmentTests, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, &shadowMapBarrier);

#pragma region Main Render Pass
	std::vector<vk::ClearValue> clearValues = { clearDepth, clearColor };

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = mainRenderPass;
	renderPassInfo.framebuffer = swapchain->GetCurrentFrame()->GetMainRenderTarget()->GetFramebuffer();
	renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	renderPassInfo.renderArea.extent = swapchain->GetExtent();
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	
	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	Resource::Material* currentMaterial = nullptr;
	Resource::MaterialInstance* currentMaterialInstance = nullptr;

	for (auto& instanceGroup : _instanceGroups)
	{
		vk::DeviceSize offset(0);

		if (currentMaterial != instanceGroup.material)
		{
			currentMaterial = instanceGroup.material;

			currentMaterial->BindMaterial(commandBuffer);

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentMaterial->GetPipelineLayout(), 0, context->GetDescriptorSetManager()->GetDescriptorSet("Render Camera"), nullptr);
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentMaterial->GetPipelineLayout(), 1, context->GetDescriptorSetManager()->GetDescriptorSet("Instance Model"), nullptr);

			//LogMat4(directionnalLight->GetViewProjectionMatrix());

			//LOG_DEBUG(MF("Switching pipeline [", currentMaterial, "]"));
		}

		if (currentMaterialInstance != instanceGroup.materialInstance)
		{
			currentMaterialInstance = instanceGroup.materialInstance;
			currentMaterialInstance->BindMaterialInstance(commandBuffer);

			//LOG_DEBUG(MF("Switching material [", currentMaterialInstance, "]"));
		}

		if (currentMesh != instanceGroup.mesh)
		{
			currentMesh = instanceGroup.mesh;
			commandBuffer.bindVertexBuffers(0, 1, &currentMesh->GetVertexBuffer(), &offset);
			commandBuffer.bindIndexBuffer(currentMesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);

			//LOG_DEBUG(MF("Switching mesh [", currentMesh, "]"));
		}

		//Helper::Memory::MapMemory(context->GetDevice(), instancedBufferMemory, sizeof(Render::TransformData) * instanceGroup.transforms.size(), instanceGroup.instanceOffset * sizeof(Render::TransformData), instanceGroup.transforms.data());

		commandBuffer.drawIndexed(instanceGroup.mesh->GetIndexCount(), instanceGroup.transforms.size(), 0, 0, instanceGroup.instanceOffset);
	}

	commandBuffer.endRenderPass();
#pragma endregion
}

void BasicRenderer::SetupPipelines()
{
	Pipeline::DescriptorSetLayoutsManager* descriptorSetLayoutsManager = context->GetDescriptorSetLayoutsManager();
	Pipeline::DescriptorSetManager* descriptorSetManager = context->GetDescriptorSetManager();
	Pipeline::PipelinesManager* pipelinesManager = context->GetPipelinesManager();
	Pipeline::LayoutsManager* layoutsManager = context->GetLayoutsManager();

#pragma region Cameras and Instance Buffer

	vk::DescriptorSetLayout cameraLayout = descriptorSetLayoutsManager->CreateDescriptorSetLayout("Render Camera",
		{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex), // Camera Space Matrix
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment), // Sunlight data
			vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex), // Light Space Matrices
			vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eSampledImage, 1, vk::ShaderStageFlagBits::eFragment), // Shadow Mapping depth map texture
			vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment) // Shadow Mapping depth map texture sampler
		});

	vk::DescriptorSetLayout instancedModelLayout = descriptorSetLayoutsManager->CreateDescriptorSetLayout("Instanced Model",
		{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex)
		});

	vk::DescriptorSetLayout shadowMapCameraLayout = descriptorSetLayoutsManager->CreateDescriptorSetLayout("Shadow Map Camera",
		{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
		});

	descriptorSetManager->CreateDescriptorSets({ "Render Camera", "Instance Model", "Shadow Map Camera" }, { cameraLayout, instancedModelLayout, shadowMapCameraLayout });

	sunLightBuffer = Helper::Memory::CreateBuffer(context->GetDevice(), context->GetPhysicalDevice(), sizeof(SunLight), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, sunLightBufferMemory);

	Pipeline::DescriptorSetUpdate shadowMapCameraUpdate = {};
	shadowMapCameraUpdate.descriptorType = vk::DescriptorType::eUniformBuffer;
	shadowMapCameraUpdate.dstBinding = 1;
	shadowMapCameraUpdate.dstArrayElement = 0;
	shadowMapCameraUpdate.descriptorCount = 1;
	shadowMapCameraUpdate.buffer = sunLightBuffer;
	shadowMapCameraUpdate.offset = 0;
	shadowMapCameraUpdate.range = sizeof(SunLight);

	descriptorSetManager->UpdateDescriptorSet("Render Camera", shadowMapCameraUpdate); // Setting Sunlight data (direction, color, cascade count)

	instancedBuffer = Helper::Memory::CreateBuffer(context->GetDevice(), context->GetPhysicalDevice(), sizeof(Render::TransformData) * MAX_RENDERABLE_ENTITIES, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, instancedBufferMemory);

	Pipeline::DescriptorSetUpdate descriptorUpdate = {};
	descriptorUpdate.descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorUpdate.dstBinding = 0;
	descriptorUpdate.dstArrayElement = 0;
	descriptorUpdate.descriptorCount = 1;
	descriptorUpdate.buffer = instancedBuffer;
	descriptorUpdate.offset = 0;
	descriptorUpdate.range = sizeof(Render::TransformData) * MAX_RENDERABLE_ENTITIES;
	descriptorSetManager->UpdateDescriptorSet("Instance Model", descriptorUpdate);

#pragma endregion

#pragma region Layouts creation

	vk::DescriptorSetLayout colorSetLayout = descriptorSetLayoutsManager->CreateDescriptorSetLayout("Color",
		{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment)
		});

	const std::vector<vk::DescriptorSetLayout> colorLayouts = { cameraLayout, instancedModelLayout, colorSetLayout };

	vk::PipelineLayout colorLayout = layoutsManager->GetOrCreateLayout(colorLayouts, {});

	vk::DescriptorSetLayout textureSetLayout = descriptorSetLayoutsManager->CreateDescriptorSetLayout("AlbedoNormal",
		{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
			vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment)
		});

	const std::vector<vk::DescriptorSetLayout> textureLayouts = { cameraLayout, instancedModelLayout, textureSetLayout };

	vk::PipelineLayout textureLayout = layoutsManager->GetOrCreateLayout(textureLayouts, {});

	const std::vector<vk::DescriptorSetLayout> depthShadowMapLayouts = { shadowMapCameraLayout, instancedModelLayout };

	vk::PushConstantRange pushConstantRange = vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry, 0, sizeof(uint32_t));

	vk::PipelineLayout depthShadowMapLayout = layoutsManager->GetOrCreateLayout(depthShadowMapLayouts, { pushConstantRange });

#pragma endregion

#pragma region DepthShadowMap Pipeline

	Pipeline::PipelineCreateData pipelineData = {};
	pipelineData.config.name = "Depth Shadow Map";

	pipelineData.descriptorSetLayouts = depthShadowMapLayouts;

	vk::Viewport* vp = new vk::Viewport;
	vp->x = 0.f;
	vp->y = 0.f;
	vp->width = 4096; //Magic number
	vp->height = 4096; //Magic number
	vp->minDepth = 0.f;
	vp->maxDepth = 1.f;

	vk::Rect2D* scisor = new vk::Rect2D;
	scisor->extent = vk::Extent2D(4096, 4096);
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
	pipelineData.createInfo.layout = depthShadowMapLayout;
	pipelineData.createInfo.renderPass = shadowMapRenderPass;
	pipelineData.createInfo.subpass = 0;
	pipelineData.createInfo.pDepthStencilState = new vk::PipelineDepthStencilStateCreateInfo(vk::PipelineDepthStencilStateCreateFlags(), VK_TRUE, VK_TRUE, vk::CompareOp::eLess);
	pipelineData.createInfo.pViewportState = new vk::PipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, vp, 1, scisor);
	pipelineData.createInfo.pRasterizationState = new vk::PipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(), VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, VK_TRUE, 5.0f, 0.0f, 3.5f, 1.0f);
	pipelineData.createInfo.pMultisampleState = new vk::PipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags(), vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
	pipelineData.createInfo.pColorBlendState = new vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, 1, colorBlendAttachment, { 0.f, 0.f, 0.f, 0.f });
	pipelineData.createInfo.pInputAssemblyState = new vk::PipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	pipelineData.createInfo.pVertexInputState = new vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(), 1, bindingDescription, 5, attributeDescriptions);

	pipelineData.shaderFile = "Shaders/ShadowMapping.spv";
	pipelineData.mains = {
		{ vk::ShaderStageFlagBits::eVertex, "vertexMain" },
		{ vk::ShaderStageFlagBits::eGeometry, "geometryMain"},
	};

	pipelinesManager->CreatePipeline(pipelineData);

#pragma endregion

#pragma region Color Pipeline
	pipelineData = {};
	pipelineData.config.name = "Colored";

	pipelineData.descriptorSetLayouts = colorLayouts;

	vp = new vk::Viewport;
	vp->x = 0.f;
	vp->y = 0.f;
	vp->width = swapchain->GetExtent().width;
	vp->height = swapchain->GetExtent().height;
	vp->minDepth = 0.f;
	vp->maxDepth = 1.f;

	scisor = new vk::Rect2D;
	scisor->extent = swapchain->GetExtent();
	scisor->offset = vk::Offset2D(0, 0);

	colorBlendAttachment = new vk::PipelineColorBlendAttachmentState;
	colorBlendAttachment->colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment->blendEnable = VK_FALSE;

	bindingDescription = new vk::VertexInputBindingDescription(0, sizeof(Resource::Vertex), vk::VertexInputRate::eVertex);

	attributeDescriptions = new vk::VertexInputAttributeDescription[5];
	attributeDescriptions[0] = vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, position));
	attributeDescriptions[1] = vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, normal));
	attributeDescriptions[2] = vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Resource::Vertex, uv));
	attributeDescriptions[3] = vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, tangent));
	attributeDescriptions[4] = vk::VertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, bitangent));

	pipelineData.createInfo = vk::GraphicsPipelineCreateInfo();
	pipelineData.createInfo.layout = colorLayout;
	pipelineData.createInfo.renderPass = mainRenderPass;
	pipelineData.createInfo.subpass = 0;
	pipelineData.createInfo.pDepthStencilState = new vk::PipelineDepthStencilStateCreateInfo(vk::PipelineDepthStencilStateCreateFlags(), VK_TRUE, VK_TRUE, vk::CompareOp::eLess);
	pipelineData.createInfo.pViewportState = new vk::PipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, vp, 1, scisor);
	pipelineData.createInfo.pRasterizationState = new vk::PipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(), VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
	pipelineData.createInfo.pMultisampleState = new vk::PipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags(), vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
	pipelineData.createInfo.pColorBlendState = new vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, 1, colorBlendAttachment, { 0.f, 0.f, 0.f, 0.f });
	pipelineData.createInfo.pInputAssemblyState = new vk::PipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	pipelineData.createInfo.pVertexInputState = new vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(), 1, bindingDescription, 5, attributeDescriptions);

	pipelineData.shaderFile = "Shaders/CNL.spv";
	pipelineData.mains = { 
		{ vk::ShaderStageFlagBits::eVertex, "vertexMain" }, 
		{ vk::ShaderStageFlagBits::eFragment, "pixelMain" } 
	};

	pipelinesManager->CreatePipeline(pipelineData);

#pragma endregion

#pragma region Texture Pipeline
	pipelineData = {};
	pipelineData.config.name = "AlbedoNormal";

	pipelineData.descriptorSetLayouts = textureLayouts;

	vp = new vk::Viewport;
	vp->x = 0.f;
	vp->y = 0.f;
	vp->width = swapchain->GetExtent().width;
	vp->height = swapchain->GetExtent().height;
	vp->minDepth = 0.f;
	vp->maxDepth = 1.f;

	scisor = new vk::Rect2D;
	scisor->extent = swapchain->GetExtent();
	scisor->offset = vk::Offset2D(0, 0);

	colorBlendAttachment = new vk::PipelineColorBlendAttachmentState;
	colorBlendAttachment->colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment->blendEnable = VK_FALSE;

	bindingDescription = new vk::VertexInputBindingDescription(0, sizeof(Resource::Vertex), vk::VertexInputRate::eVertex);

	attributeDescriptions = new vk::VertexInputAttributeDescription[5];
	attributeDescriptions[0] = vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, position));
	attributeDescriptions[1] = vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, normal));
	attributeDescriptions[2] = vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Resource::Vertex, uv));
	attributeDescriptions[3] = vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, tangent));
	attributeDescriptions[4] = vk::VertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Resource::Vertex, bitangent));

	pipelineData.createInfo = vk::GraphicsPipelineCreateInfo();
	pipelineData.createInfo.layout = textureLayout;
	pipelineData.createInfo.renderPass = mainRenderPass;
	pipelineData.createInfo.subpass = 0;
	pipelineData.createInfo.pDepthStencilState = new vk::PipelineDepthStencilStateCreateInfo(vk::PipelineDepthStencilStateCreateFlags(), VK_TRUE, VK_TRUE, vk::CompareOp::eLess);
	pipelineData.createInfo.pViewportState = new vk::PipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, vp, 1, scisor);
	pipelineData.createInfo.pRasterizationState = new vk::PipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(), VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
	pipelineData.createInfo.pMultisampleState = new vk::PipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags(), vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
	pipelineData.createInfo.pColorBlendState = new vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, 1, colorBlendAttachment, { 0.f, 0.f, 0.f, 0.f });
	pipelineData.createInfo.pInputAssemblyState = new vk::PipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	pipelineData.createInfo.pVertexInputState = new vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(), 1, bindingDescription, 5, attributeDescriptions);

	pipelineData.shaderFile = "Shaders/TNL.spv";
	pipelineData.mains = {
		{ vk::ShaderStageFlagBits::eVertex, "vertexMain" },
		{ vk::ShaderStageFlagBits::eFragment, "pixelMain" }
	};

	pipelinesManager->CreatePipeline(pipelineData);
#pragma endregion
	//TODO : Introduce pipeline cache
}

void BasicRenderer::UpdateRenderCameraBuffer(const vk::Buffer& _buffer)
{
	Pipeline::DescriptorSetUpdate cameraUpdate = {};
	cameraUpdate.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraUpdate.dstBinding = 0;
	cameraUpdate.dstArrayElement = 0;
	cameraUpdate.descriptorCount = 1;
	cameraUpdate.buffer = _buffer;
	cameraUpdate.offset = 0;
	cameraUpdate.range = sizeof(CameraUBO);

	context->GetDescriptorSetManager()->UpdateDescriptorSet("Render Camera", cameraUpdate);
}

void BasicRenderer::SetupDirectionalLight(const vk::Extent2D _extent, const glm::vec4& _color, const glm::vec3& _direction, const uint32_t& _cascadeCount, const float* _splits)
{
	shadowMapRT = new Render::RenderTarget(*context, _extent);

	uint32_t actualCascadeCount = _cascadeCount;

	if (_cascadeCount > MAX_CASCADE_COUNT)
	{
		LOG_WARNING(MF("Cascade count is too high, setting it to ", MAX_CASCADE_COUNT));
		actualCascadeCount = MAX_CASCADE_COUNT;
	}

	auto shadowMapAttachment = std::make_shared<Render::RenderTargetAttachment>(context, _extent,
		Helper::Format::FindDepthFormat(context->GetPhysicalDevice()), vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eDepth, true, actualCascadeCount);

	shadowMapRT->AddAttachment(shadowMapAttachment);

	shadowMapRT->Build(shadowMapRenderPass, actualCascadeCount);

	sunLight.lightColor = _color;
	sunLight.lightDirection = glm::vec4(_direction, 0.f);
	sunLight.cascadeCount = actualCascadeCount;

	for (int i = 0; i < actualCascadeCount; i++)
	{
		shadowMapCascades.viewProjectionMatrix[i] = glm::mat4(1.f);
		shadowMapCascades.splitDepth[i] = _splits[i];
		LOG_DEBUG("Split depth : " + std::to_string(shadowMapCascades.splitDepth[i]));
	}

	shadowMapCascadesBuffer = Helper::Memory::CreateBuffer(context->GetDevice(), context->GetPhysicalDevice(), sizeof(ShadowMapCascades), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, shadowMapCascadesBufferMemory);
	Helper::Memory::MapMemory(context->GetDevice(), shadowMapCascadesBufferMemory, sizeof(ShadowMapCascades), 0, &shadowMapCascades);

	Pipeline::DescriptorSetUpdate shadowMapVPMatricesUpdate = {};
	shadowMapVPMatricesUpdate.descriptorType = vk::DescriptorType::eUniformBuffer;
	shadowMapVPMatricesUpdate.dstBinding = 0;
	shadowMapVPMatricesUpdate.dstArrayElement = 0;
	shadowMapVPMatricesUpdate.descriptorCount = 1;
	shadowMapVPMatricesUpdate.buffer = shadowMapCascadesBuffer;
	shadowMapVPMatricesUpdate.offset = 0;
	shadowMapVPMatricesUpdate.range = sizeof(ShadowMapCascades);

	context->GetDescriptorSetManager()->UpdateDescriptorSet("Shadow Map Camera", shadowMapVPMatricesUpdate);

	shadowMapVPMatricesUpdate.dstBinding = 2;
	context->GetDescriptorSetManager()->UpdateDescriptorSet("Render Camera", shadowMapVPMatricesUpdate);

	Pipeline::DescriptorSetUpdate shadowMapUpdate = {};
	shadowMapUpdate.updateType = Pipeline::DescriptorSetUpdateType::IMAGE;
	shadowMapUpdate.descriptorType = vk::DescriptorType::eSampledImage;
	shadowMapUpdate.dstBinding = 3;
	shadowMapUpdate.dstArrayElement = 0;
	shadowMapUpdate.descriptorCount = 1;
	shadowMapUpdate.sampler = shadowMapRT->GetAttachment(0)->GetSampler();
	shadowMapUpdate.imageView = shadowMapRT->GetAttachment(0)->GetImageView();
	shadowMapUpdate.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	context->GetDescriptorSetManager()->UpdateDescriptorSet("Render Camera", shadowMapUpdate); // Setting Shadow Map texture array
	shadowMapUpdate.descriptorType = vk::DescriptorType::eSampler;
	shadowMapUpdate.dstBinding = 4;
	context->GetDescriptorSetManager()->UpdateDescriptorSet("Render Camera", shadowMapUpdate); // Setting Shadow Map texture sampler
}

void BasicRenderer::UpdateDirectionalLight(const glm::mat4* _lightViewProj)
{
	for (int i = 0; i < sunLight.cascadeCount; i++)
	{
		shadowMapCascades.viewProjectionMatrix[i] = _lightViewProj[i];
	}

	Helper::Memory::MapMemory(context->GetDevice(), shadowMapCascadesBufferMemory, sizeof(ShadowMapCascades), 0, &shadowMapCascades);
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

void BasicRenderer::CreateRenderPasses()
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

	vk::AttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription depthSubpass = {};
	depthSubpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	depthSubpass.colorAttachmentCount = 0;
	depthSubpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::vector<vk::AttachmentDescription> attachments = { depthAttachment };
	std::vector<vk::SubpassDescription> subpasses = { depthSubpass };

	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();

	shadowMapRenderPass = context->GetDevice().createRenderPass(renderPassInfo);

	
}

void LogMat4(glm::mat4 _mat)
{
	LOG_DEBUG(MF(_mat[0][0], " ", _mat[0][1], " ", _mat[0][2], " ", _mat[0][3]));
	LOG_DEBUG(MF(_mat[1][0], " ", _mat[1][1], " ", _mat[1][2], " ", _mat[1][3]));
	LOG_DEBUG(MF(_mat[2][0], " ", _mat[2][1], " ", _mat[2][2], " ", _mat[2][3]));
	LOG_DEBUG(MF(_mat[3][0], " ", _mat[3][1], " ", _mat[3][2], " ", _mat[3][3]));
}
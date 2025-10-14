#include "pch.hpp"

#include "Material.hpp"
#include "Render/Renderer/Renderer.hpp"
#include "MaterialInstance.hpp"

#include "Util/Serializers/ISerializer.hpp"

std::unordered_map<cp::MaterialFieldType, size_t> cp::Material::MaterialFieldSizeMap = {
	{ cp::MaterialFieldType::Bool, sizeof(bool) },
	{ cp::MaterialFieldType::Half, sizeof(float) },
	{ cp::MaterialFieldType::Float, sizeof(double) },
	{ cp::MaterialFieldType::Int, sizeof(int) },
	{ cp::MaterialFieldType::UInt, sizeof(unsigned int) },
	{ cp::MaterialFieldType::Vector, sizeof(glm::vec4) },
	{ cp::MaterialFieldType::Matrix, sizeof(glm::mat4) },
};

cp::Material::Material(const cp::VulkanContext* _context) :
	context(_context)
{
	AddShaderStage(cp::ShaderStages::Vertex);
	AddShaderStage(cp::ShaderStages::Fragment);
}

cp::Material::~Material()
{
	for (auto& [name, pipelineData] : pipelineDatas)
	{
		if (rpRequirements.at(name).useDefaultShader) continue; // Skip if the material is using the default shader

		if (pipelineData->pipelineLayout) context->GetLayoutsManager()->UnloadLayout(pipelineData->pipelineLayout); // Unload the previous layout if it exists
		if (pipelineData->pipeline) context->GetPipelinesManager()->DestroyPipeline({ this->moduleName + "_" + name }); // Unload the previous pipeline if it exists
	}

	//for (auto& [name, desc] : descriptors)
	//{
	//	if(desc.layout) context->GetDevice().destroyDescriptorSetLayout(desc.layout); // Destroy the descriptor set layout
	//}
}

void cp::Material::BindMaterial(vk::CommandBuffer& _command)
{
	//_command.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData->pipeline);
}

void cp::Material::Reload(cp::Renderer& _renderer)
{
	auto ValueOrDefault = [](const std::string& str, const std::string& defaultValue) { return str.empty() ? defaultValue : str; }; // This lambda is used to check if the string is empty and return the default value if it is

	// Since this function in called when a material is loaded in the scene and needs to be reloaded
	// we can generate the descriptor set layouts and pipeline layout here

	//for (auto& [name, desc] : descriptors)
	//{
	//	desc.GenerateDescriptorSetLayout(context); //This (re)generate the descriptor set layout
	//}

	CreateDescriptorSetLayouts(); // Create the descriptor set layouts based on the current descriptors

	cp::LayoutsManager* layoutsManager = context->GetLayoutsManager();
	cp::PipelinesManager* pipelinesManager = context->GetPipelinesManager();

	for (auto& [name, rpRequirement] : rpRequirements)
	{
		if (!rpRequirement.renderToPass) continue; // Skip if the material is not rendered in the renderpass
		// TODO : For now it's okay but we should check if pipelineDatas[moduleName] exists, we should unload the previous data if it now doesn't render to the pass
		if (rpRequirement.useDefaultShader && _renderer.GetRenderPass(name).GetDefaultPipeline().has_value())
		{
			pipelineDatas.insert({ name, _renderer.GetRenderPass(name).GetDefaultPipeline().value() }); // Store the renderpass default pipeline if it's the one being used by the Material for that pass
			continue; // Skip if the material is using the default shader
		}

		if (rpRequirement.customShaderPath.empty())
		{
			LOG_WARNING(MF("Custom shader path is empty for material [", name, "]"));
			continue; // Skip if the custom shader path is empty
		}

		if (pipelineDatas.find(name) != pipelineDatas.end()) // Check if the pipeline data already exists
		{
			//In this case, we unload the previous layout and pipeline
			if (pipelineDatas[name]->pipelineLayout) layoutsManager->UnloadLayout(pipelineDatas[name]->pipelineLayout); // Unload the previous layout if it exists
			if (pipelineDatas[name]->pipeline) pipelinesManager->DestroyPipeline({ this->moduleName + "_" + name }); // Unload the previous pipeline if it exists
		}

		vk::PipelineLayout pipelineLayout = layoutsManager->GetOrCreateLayout(descriptorSetLayouts, {}); // Create the new layout // TODO : Add PushConstants support

		cp::PipelineCreateData pipelineCreateData;
		pipelineCreateData.config = { this->moduleName + "_" + name }; // Create a unique moduleName for the pipeline
		pipelineCreateData.createInfo.layout = pipelineLayout;
		pipelineCreateData.createInfo.renderPass = _renderer.GetRenderPass(name).GetRenderPass();
		pipelineCreateData.shaderFile = rpRequirement.customShaderPath;
		
		pipelineCreateData.mains = { 
			{ vk::ShaderStageFlagBits::eVertex, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Vertex], "Vertex_Default") },
			{ vk::ShaderStageFlagBits::eFragment, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Fragment], "Fragment_Default") },
			{ vk::ShaderStageFlagBits::eGeometry, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Geometry], "Geometry_Default") },
			{ vk::ShaderStageFlagBits::eTessellationControl, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::TessellationControl], "TessellationControl_Default") },
			{ vk::ShaderStageFlagBits::eTessellationEvaluation, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::TessellationEvaluation], "TessellationEvaluation_Default") },
			{ vk::ShaderStageFlagBits::eMeshEXT, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Mesh], "Mesh_Default") },
			{ vk::ShaderStageFlagBits::eCompute, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Compute], "Compute_Default") }
		};
		pipelineCreateData.descriptorSetLayouts = descriptorSetLayouts;

		std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

		vk::PipelineColorBlendAttachmentState* colorBlendAttachment = new vk::PipelineColorBlendAttachmentState;
		colorBlendAttachment->colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment->blendEnable = pipelineCreationData.enableBlending;
		colorBlendAttachment->srcColorBlendFactor = pipelineCreationData.srcColorBlendFactor;
		colorBlendAttachment->dstColorBlendFactor = pipelineCreationData.dstColorBlendFactor;
		colorBlendAttachment->colorBlendOp = pipelineCreationData.colorBlendOp;
		colorBlendAttachment->srcAlphaBlendFactor = pipelineCreationData.srcAlphaBlendFactor;
		colorBlendAttachment->dstAlphaBlendFactor = pipelineCreationData.dstAlphaBlendFactor;
		colorBlendAttachment->alphaBlendOp = pipelineCreationData.alphaBlendOp;

		vk::VertexInputBindingDescription* bindingDescription = new vk::VertexInputBindingDescription(0, sizeof(cp::Vertex), vk::VertexInputRate::eVertex);

		vk::VertexInputAttributeDescription* attributeDescriptions = new vk::VertexInputAttributeDescription[5];
		attributeDescriptions[0] = vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(cp::Vertex, position));
		attributeDescriptions[1] = vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(cp::Vertex, normal));
		attributeDescriptions[2] = vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(cp::Vertex, uv));
		attributeDescriptions[3] = vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(cp::Vertex, tangent));
		attributeDescriptions[4] = vk::VertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32Sfloat, offsetof(cp::Vertex, bitangent));

		pipelineCreateData.createInfo.subpass = 0; // TODO IMPORTANT : Add support for multiple subpasses
		pipelineCreateData.createInfo.pDynamicState = new vk::PipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data());
		pipelineCreateData.createInfo.pDepthStencilState = new vk::PipelineDepthStencilStateCreateInfo(vk::PipelineDepthStencilStateCreateFlags(), pipelineCreationData.depthTestEnable, pipelineCreationData.depthWriteEnable, pipelineCreationData.depthCompareOp);
		pipelineCreateData.createInfo.pViewportState = new vk::PipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);
		pipelineCreateData.createInfo.pRasterizationState = new vk::PipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(), VK_FALSE, VK_FALSE, pipelineCreationData.polygonMode, pipelineCreationData.cullMode, pipelineCreationData.frontFace, pipelineCreationData.depthBiasEnable, pipelineCreationData.depthBiasConstantFactor, pipelineCreationData.depthBiasClamp, pipelineCreationData.depthBiasSlopeFactor, 1.0f);
		pipelineCreateData.createInfo.pMultisampleState = new vk::PipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags(), pipelineCreationData.rasterizationSamples, pipelineCreationData.sampleShadingEnable, 1.0f, nullptr, VK_FALSE, VK_FALSE);
		pipelineCreateData.createInfo.pColorBlendState = new vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, 1, colorBlendAttachment, { 0.f, 0.f, 0.f, 0.f });
		pipelineCreateData.createInfo.pInputAssemblyState = new vk::PipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE);
		pipelineCreateData.createInfo.pVertexInputState = new vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(), 1, bindingDescription, 5, attributeDescriptions);

		//pipelineCreateData.createInfo.pTessellationState = new vk::PipelineTessellationStateCreateInfo(vk::PipelineTessellationStateCreateFlags(), 3); // TODO : Add support for tessellation

		pipelineDatas.insert({ name, &pipelinesManager->CreatePipeline(pipelineCreateData) }); // Create the pipeline and store it in the map
	}
}

void cp::Material::Serialize(cp::ISerializer& _serializer) const
{
	_serializer.WriteString("Name", moduleName);
	_serializer.WriteString("ShaderPath", shaderPath);
	_serializer.WriteInt("Shader Stages", static_cast<int>(shaderStages));

	_serializer.BeginObjectArrayWriting("RenderPass Requirements");

	for (const auto& [name, rpr] : rpRequirements)
	{
		_serializer.BeginObjectArrayElementWriting();
		_serializer.WriteString("RP Name", name);
		_serializer.BeginObjectWriting("Requirements");
		rpr.Serialize(_serializer);
		_serializer.EndObject();
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();

	if (shaderReflection) // Check if the shader reflection exists
	{
		_serializer.BeginObjectWriting("ShaderReflection");
		shaderReflection->Serialize(_serializer); // Serialize the shader reflection
		_serializer.EndObject();
	}
	else
	{
		LOG_WARNING(MF("Shader reflection is null for material [", moduleName, "]"));
	}
}

void cp::Material::Deserialize(ISerializer& _serializer)
{
	moduleName = _serializer.ReadString("Name", "Unknown");
	shaderPath = _serializer.ReadString("ShaderPath", "");
	shaderStages = static_cast<uint16_t>(_serializer.ReadInt("Shader Stages", 0));

	size_t elements = _serializer.BeginObjectArrayReading("RenderPass Requirements");

	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		std::string rpName = _serializer.ReadString("RP Name", "Error");

		if (rpName == "Error")
		{
			_serializer.EndObjectArrayElement();
			continue;
		}

		RenderPassRequirement rpr;
		_serializer.BeginObjectReading("Requirements");
		rpr.Deserialize(_serializer);
		_serializer.EndObject();
		rpRequirements[rpName] = rpr;
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();

	if (shaderReflection) delete shaderReflection; // Delete the previous shader reflection if it exists

	shaderReflection = new ShaderReflection(); // Create a new shader reflection
	_serializer.BeginObjectReading("ShaderReflection");
	shaderReflection->Deserialize(_serializer); // Deserialize the shader reflection
	_serializer.EndObject();
}

std::vector<std::string> cp::Material::GetUniqueEntryPoints() const
{
	std::vector<std::string> entryPoints;

	for (const auto& [name, rpRequirement] : rpRequirements)
	{
		if (rpRequirement.renderToPass && !rpRequirement.useDefaultShader)
		{
			for (const auto& [stage, entryPoint] : rpRequirement.customEntryPoints)
			{
				if (std::find(entryPoints.begin(), entryPoints.end(), entryPoint) == entryPoints.end())
					entryPoints.push_back(entryPoint);
			}
		}
	}

	return entryPoints;
}

void cp::Material::CreateDescriptorSetLayouts()
{
	if (descriptorSetLayouts.size() > 0)
	{
		LOG_WARNING("Material already has descriptor set layouts, they will be destroyed");
		for (auto& layout : descriptorSetLayouts)
		{
			context->GetDevice().destroyDescriptorSetLayout(layout); // Destroy the previous descriptor set layout
		}
		descriptorSetLayouts.clear();
	}

	std::sort(shaderReflection->resources.begin(), shaderReflection->resources.end(), [](const ShaderResource& a, const ShaderResource& b) {
		return a.set < b.set && a.binding < b.binding;
		});

	size_t lastSetIndex = ~(0ull);

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	std::vector<vk::DescriptorSetLayoutBinding> bindings;

	for (const auto& resource : shaderReflection->resources)
	{
		if (lastSetIndex != resource.set)
		{
			if (lastSetIndex != ~(0ull)) // If this is not the first set
			{
				layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // Set the binding count for the layout info
				layoutInfo.pBindings = bindings.data(); // Set the bindings for the layout info
				descriptorSetLayouts.push_back(context->GetDevice().createDescriptorSetLayout(layoutInfo)); // Create the descriptor set layout for the previous set
				bindings.clear(); // Clear the bindings for the next set
			}

			lastSetIndex = resource.set; // Update the last set index
			layoutInfo = vk::DescriptorSetLayoutCreateInfo(); // Reset the layout info
		}

		vk::DescriptorSetLayoutBinding binding;
		binding.binding = resource.binding;
		binding.descriptorType = Helper::Material::GetDescriptorTypeFromBindingType(resource.kind);
		binding.descriptorCount = 1; // For now we only support one descriptor per binding
		binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics; // For now it's all graphics, we need to change it later to send binding only the stages that are used by the material
		bindings.push_back(binding); // Add the binding to the list
	}

	if (lastSetIndex != ~(0ull)) // If there is still a set to create
	{
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // Set the binding count for the layout info
		layoutInfo.pBindings = bindings.data(); // Set the bindings for the layout info
		descriptorSetLayouts.push_back(context->GetDevice().createDescriptorSetLayout(layoutInfo)); // Create the descriptor set layout for the last set
	}

	if (descriptorSetLayouts.empty())
	{
		LOG_WARNING("No descriptor set layouts were created for the material, this is likely an error in the shader reflection");
	}
}

void cp::RenderPassRequirement::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteBool("ShouldRenderToPass", renderToPass);
	_serializer.WriteBool("UseDefaultShader", useDefaultShader);

	if (!customEntryPoints.empty())
	{
		_serializer.BeginObjectWriting("CustomEntryPoints");

		for (const auto& [stage, entryPoint] : customEntryPoints)
		{
			switch (stage)
			{
			case ShaderStages::Vertex:
				_serializer.WriteString("VertexEntryPoint", entryPoint);
				break;
			case ShaderStages::Fragment:
				_serializer.WriteString("FragmentEntryPoint", entryPoint);
				break;
			case ShaderStages::Geometry:
				_serializer.WriteString("GeometryEntryPoint", entryPoint);
				break;
			case ShaderStages::TessellationControl:
				_serializer.WriteString("TCEntryPoint", entryPoint);
				break;
			case ShaderStages::TessellationEvaluation:
				_serializer.WriteString("TEEntryPoint", entryPoint);
				break;
			case ShaderStages::Mesh:
				_serializer.WriteString("MeshEntryPoint", entryPoint);
				break;
			case ShaderStages::Compute:
				_serializer.WriteString("ComputeEntryPoint", entryPoint);
				break;
			}
		}

		_serializer.EndObject();
	}

	if (!customShaderPath.empty()) _serializer.WriteString("CustomShaderPath", customShaderPath);
}

void cp::RenderPassRequirement::Deserialize(ISerializer& _serializer)
{
	renderToPass = _serializer.ReadBool("ShouldRenderToPass", false);
	useDefaultShader = _serializer.ReadBool("UseDefaultShader", true);

	if (_serializer.BeginObjectReading("CustomEntryPoints"))
	{
		std::string entryPoint;
		if (entryPoint = _serializer.ReadString("VertexEntryPoint", ""); !entryPoint.empty()) customEntryPoints[ShaderStages::Vertex] = entryPoint;
		if (entryPoint = _serializer.ReadString("FragmentEntryPoint", ""); !entryPoint.empty()) customEntryPoints[ShaderStages::Fragment] = entryPoint;
		if (entryPoint = _serializer.ReadString("GeometryEntryPoint", ""); !entryPoint.empty()) customEntryPoints[ShaderStages::Geometry] = entryPoint;
		if (entryPoint = _serializer.ReadString("TCEntryPoint", ""); !entryPoint.empty()) customEntryPoints[ShaderStages::TessellationControl] = entryPoint;
		if (entryPoint = _serializer.ReadString("TEEntryPoint", ""); !entryPoint.empty()) customEntryPoints[ShaderStages::TessellationEvaluation] = entryPoint;
		if (entryPoint = _serializer.ReadString("MeshEntryPoint", ""); !entryPoint.empty()) customEntryPoints[ShaderStages::Mesh] = entryPoint;
		if (entryPoint = _serializer.ReadString("ComputeEntryPoint", ""); !entryPoint.empty()) customEntryPoints[ShaderStages::Compute] = entryPoint;

		_serializer.EndObject();
	}

	customShaderPath = _serializer.ReadString("CustomShaderPath", "");
}
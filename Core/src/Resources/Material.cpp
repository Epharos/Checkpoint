#include "pch.hpp"

#include "Material.hpp"
#include "Render/Renderer/Renderer.hpp"

#include "Util/Serializers/ISerializer.hpp"

std::unordered_map<cp::MaterialFieldType, size_t> cp::Material::MaterialFieldSizeMap = {
	{ cp::MaterialFieldType::BOOL, sizeof(bool) },
	{ cp::MaterialFieldType::HALF, sizeof(float) },
	{ cp::MaterialFieldType::FLOAT, sizeof(double) },
	{ cp::MaterialFieldType::INT, sizeof(int) },
	{ cp::MaterialFieldType::UINT, sizeof(unsigned int) },
	{ cp::MaterialFieldType::VECTOR, sizeof(glm::vec4) },
	{ cp::MaterialFieldType::MATRIX, sizeof(glm::mat4) },
};

cp::Material::Material(const cp::VulkanContext* _context) :
	context(_context)
{
	AddShaderStage(cp::ShaderStages::Vertex);
	AddShaderStage(cp::ShaderStages::Fragment);
}

void cp::Material::BindMaterial(vk::CommandBuffer& _command)
{
	//_command.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData->pipeline);
}

void cp::Material::Reload(cp::Renderer& _renderer)
{
	auto ValueOrDefault = [](const std::string& str, const std::string& defaultValue) { return str.empty() ? defaultValue : str; }; // This function is used to check if the string is empty and return the default value if it is

	// Since this function in called when a material is loaded in the scene and needs to be reloaded
	// we can generate the descriptor set layouts and pipeline layout here

	for (auto& desc : descriptors)
	{
		desc.GenerateDescriptorSetLayout(context); //This (re)generate the descriptor set layout
	}

	std::vector<vk::DescriptorSetLayout> layouts;
	for (auto& desc : descriptors)
		layouts.push_back(desc.layout);

	cp::LayoutsManager* layoutsManager = context->GetLayoutsManager();
	cp::PipelinesManager* pipelinesManager = context->GetPipelinesManager();

	for (auto& [name, rpRequirement] : rpRequirements)
	{
		if (!rpRequirement.renderToPass) continue; // Skip if the material is not rendered in the renderpass
		// TODO : For now it's okay but we should check if pipelineDatas[name] exists, we should unload the previous data if it now doesn't render to the pass
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
			if (pipelineDatas[name]->pipeline) pipelinesManager->DestroyPipeline({ this->name + "_" + name }); // Unload the previous pipeline if it exists
		}

		vk::PipelineLayout pipelineLayout = layoutsManager->GetOrCreateLayout(layouts, {}); // Create the new layout // TODO : Add PushConstants support

		cp::PipelineCreateData pipelineCreateData;
		pipelineCreateData.config = { this->name + "_" + name }; // Create a unique name for the pipeline
		pipelineCreateData.createInfo.layout = pipelineLayout;
		pipelineCreateData.createInfo.renderPass = _renderer.GetRenderPass(name).GetRenderPass();
		pipelineCreateData.shaderFile = rpRequirement.customShaderPath;
		
		pipelineCreateData.mains = { 
			{ vk::ShaderStageFlagBits::eVertex, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Vertex], "Vertex_Default") },
			{ vk::ShaderStageFlagBits::eFragment, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Fragment], "Fragment_Default") },
			{ vk::ShaderStageFlagBits::eGeometry, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Geometry], "Geometry_Default") },
			{ vk::ShaderStageFlagBits::eTessellationControl, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::TessellationControl], "TessellationControl_Default") },
			{ vk::ShaderStageFlagBits::eTessellationEvaluation, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::TessellationEvaluation], "TessellationEvaluation_Default") },
			{ vk::ShaderStageFlagBits::eMeshNV, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Mesh], "Mesh_Default") },
			{ vk::ShaderStageFlagBits::eCompute, ValueOrDefault(rpRequirement.customEntryPoints[cp::ShaderStages::Compute], "Compute_Default") }
		};
		pipelineCreateData.descriptorSetLayouts = layouts;

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
	_serializer.WriteString("Name", name);
	_serializer.WriteString("ShaderPath", shaderPath);
	_serializer.WriteInt("Shader Stages", static_cast<int>(shaderStages));

	_serializer.BeginObjectArrayWriting("Descriptors");

	for (const MaterialDescriptor& desc : descriptors)
	{
		_serializer.BeginObjectArrayElementWriting();
		desc.Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();

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
}

void cp::Material::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Name", "Unknown");
	shaderPath = _serializer.ReadString("ShaderPath", "");
	shaderStages = static_cast<uint16_t>(_serializer.ReadInt("Shader Stages", 0));

	size_t elements = _serializer.BeginObjectArrayReading("Descriptors");

	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		MaterialDescriptor desc;
		desc.Deserialize(_serializer);

		descriptors.push_back(desc);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();

	elements = _serializer.BeginObjectArrayReading("RenderPass Requirements");

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

void cp::MaterialDescriptor::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteInt("Index", index);
	_serializer.WriteString("Name", name);

	_serializer.BeginObjectArrayWriting("Bindings");
	for (const MaterialBinding& binding : bindings)
	{
		_serializer.BeginObjectArrayElementWriting();
		binding.Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}
	_serializer.EndObjectArray();
}

void cp::MaterialDescriptor::Deserialize(ISerializer& _serializer)
{
	index = _serializer.ReadInt("Index", 0);
	name = _serializer.ReadString("Name", "Error");

	size_t elements = _serializer.BeginObjectArrayReading("Bindings");

	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		MaterialBinding binding;
		binding.Deserialize(_serializer);
		bindings.push_back(binding);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

void cp::MaterialDescriptor::GenerateDescriptorSetLayout(const cp::VulkanContext* _context)
{
	if (layout)
	{
		_context->GetDevice().destroyDescriptorSetLayout(layout);
		layout = VK_NULL_HANDLE;
	}

	std::vector<vk::DescriptorSetLayoutBinding> bindings;

	for (const MaterialBinding& binding : this->bindings)
	{
		vk::DescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = binding.index;
		layoutBinding.descriptorType = Helper::Material::GetDescriptorTypeFromBindingType(binding.type);
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = Helper::Material::GetShaderStageFlags(binding.shaderStages);
		bindings.push_back(layoutBinding);
	}

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	layout = _context->GetDevice().createDescriptorSetLayout(layoutInfo);
}

bool cp::MaterialDescriptor::RemoveBinding(const MaterialBinding& _binding)
{
	auto it = std::remove_if(bindings.begin(), bindings.end(), [&_binding](const MaterialBinding& binding) { return &_binding == &binding; });
	return it != bindings.end() ? (bindings.erase(it), true) : false;
}

void cp::MaterialBinding::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteInt("Type", (int)type);
	_serializer.WriteInt("Index", index);
	_serializer.WriteInt("ShaderStage", shaderStages);
	_serializer.WriteString("Name", name);

	_serializer.BeginObjectArrayWriting("Fields");
	for (const MaterialField& field : fields)
	{
		_serializer.BeginObjectArrayElementWriting();
		field.Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}
	_serializer.EndObjectArray();
}

void cp::MaterialBinding::Deserialize(ISerializer& _serializer)
{
	type = (BindingType)_serializer.ReadInt("Type", 0);
	index = _serializer.ReadInt("Index", 0);
	shaderStages = _serializer.ReadInt("ShaderStage", 0);
	name = _serializer.ReadString("Name", "Error");

	size_t elements = _serializer.BeginObjectArrayReading("Fields");
	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		MaterialField field;
		field.Deserialize(_serializer);
		fields.push_back(field);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

bool cp::MaterialBinding::RemoveField(const MaterialField& _field)
{
	auto it = std::remove_if(fields.begin(), fields.end(), [&_field](const MaterialField& field) { return &_field == &field; });
	
	return it != fields.end() ? (fields.erase(it), true) : false;
}

void cp::MaterialField::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteInt("Type", (int)type);
	_serializer.WriteString("Name", name);
	_serializer.WriteInt("Offset", offset);
}

void cp::MaterialField::Deserialize(ISerializer& _serializer)
{
	type = (MaterialFieldType)_serializer.ReadInt("Type", 0);
	name = _serializer.ReadString("Name", "Error");
	offset = _serializer.ReadInt("Offset", 0);
}

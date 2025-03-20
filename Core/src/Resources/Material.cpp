#include "pch.hpp"

#include "Material.hpp"

#include "Util/Serializers/ISerializer.hpp"

std::unordered_map<cp::MaterialFieldType, size_t> cp::Material::MaterialFieldSizeMap = {
	{ cp::MaterialFieldType::BOOL, sizeof(bool) },
	{ cp::MaterialFieldType::FLOAT, sizeof(float) },
	{ cp::MaterialFieldType::INT, sizeof(int) },
	{ cp::MaterialFieldType::VEC2, sizeof(glm::vec2) },
	{ cp::MaterialFieldType::VEC3, sizeof(glm::vec3) },
	{ cp::MaterialFieldType::VEC4, sizeof(glm::vec4) },
	{ cp::MaterialFieldType::MAT4, sizeof(glm::mat4) }
};

std::vector<vk::DescriptorSetLayoutBinding> cp::Material::GenerateBindings()
{
	//TODO : Change shader stage to what's relevant (user input)
	//TODO : Allow user to define descriptor sets, uniform buffers, storage buffers, ...

	std::vector<vk::DescriptorSetLayoutBinding> bindings;

	bindings.push_back(vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics)); //Camera data

	uint8_t textureIndex = 1;

	for (auto mf : fields)
	{
		if (mf.type == MaterialFieldType::TEXTURE)
			bindings.push_back(vk::DescriptorSetLayoutBinding(textureIndex++, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eAllGraphics));
	}

	return bindings;
}

cp::Material::Material(const cp::VulkanContext* _context) :
	context(_context)
{

}

void cp::Material::BindMaterial(vk::CommandBuffer& _command)
{
	//_command.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData->pipeline);
}

void cp::Material::Reload()
{
	context->GetDescriptorSetLayoutsManager()->OverrideDescriptorSetLayout(name, GenerateBindings());
}

void cp::Material::Serialize(cp::ISerializer& _serializer) const
{
	//TODO: Allow user to define their own descriptor sets (layouts)

	_serializer.WriteString("Name", name);

	_serializer.BeginObjectArrayWriting("Fields");

	for (const MaterialField& field : fields)
	{
		_serializer.BeginObjectArrayElementWriting();
		_serializer.WriteInt("Type", static_cast<int>(field.type));
		_serializer.WriteString("Name", field.name);
		_serializer.WriteInt("Offset", field.offset);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

void cp::Material::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Name", "Unknown");

	size_t elements = _serializer.BeginObjectArrayReading("Fields");

	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		MaterialField field;
		field.type = static_cast<MaterialFieldType>(_serializer.ReadInt("Type", 0));
		field.name = _serializer.ReadString("Name", "Error");
		field.offset = _serializer.ReadInt("Offset", 0);
		fields.push_back(field);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

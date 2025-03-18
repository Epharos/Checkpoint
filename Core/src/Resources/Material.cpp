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

cp::Material::Material(const cp::PipelineData& _pipeline, const vk::DescriptorSetLayout& _descriptorSetLayout, const cp::VulkanContext* _context) :
	pipelineData(&_pipeline), descriptorSetLayout(_descriptorSetLayout), context(_context)
{

}

void cp::Material::BindMaterial(vk::CommandBuffer& _command)
{
	_command.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineData->pipeline);
}

void cp::Material::Serialize(cp::ISerializer& _serializer) const
{
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

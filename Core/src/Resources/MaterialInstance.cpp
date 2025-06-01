#include "pch.hpp"
#include "MaterialInstance.hpp"

#include "Material.hpp"

#include "Util/Serializers/JsonSerializer.hpp"
#include "ResourceManager.hpp"

#ifdef IN_EDITOR
#include "../Editor/ResourceDropLineEdit.hpp"
#endif

cp::MaterialInstance::MaterialInstance(const cp::VulkanContext* _context)
{
	context = _context;
}

cp::MaterialInstance::~MaterialInstance()
{
	
}

void cp::MaterialInstance::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Associated Material", associatedMaterial);

	if (associatedMaterial.empty())
	{
		LOG_ERROR("Associated material is empty");
		return;
	}

	_serializer.BeginObjectArrayWriting("Resources");
	for (const auto& resource : resources)
	{
		_serializer.BeginObjectArrayElementWriting();
		resource.Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}
	_serializer.EndObjectArray();
}

void cp::MaterialInstance::Deserialize(ISerializer& _serializer)
{
	associatedMaterial = _serializer.ReadString("Associated Material", "");

	if (associatedMaterial.empty())
	{
		LOG_ERROR("Associated material is empty");
		return;
	}

	material = cp::ResourceManager::Get()->Get<Material>(associatedMaterial);

	if (!material) //If the material is not loaded, we need to load it
	{
		cp::JsonSerializer matSerializer;
		matSerializer.Read(associatedMaterial);
		material = std::make_shared<Material>(context);
		material->Deserialize(matSerializer);
	}

	if (!material)
	{
		LOG_ERROR("MaterialInstance: Material is null");
		return;
	}

	size_t elements = _serializer.BeginObjectArrayReading("Resources");
	
	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		MaterialInstanceResource resource(context);
		resource.Deserialize(_serializer);
		resources.push_back(resource);
		_serializer.EndObjectArrayElement();
	}

	ValidateData();
}

std::vector<cp::MaterialInstanceResource> cp::MaterialInstance::CreateMaterialInstanceResources()
{
	std::vector<cp::MaterialInstanceResource> newResources;

	if (!material || !material->GetShaderReflection())
	{
		LOG_ERROR("Material or shader reflection is null");
		return newResources;
	}

	for (const auto& resource : material->GetShaderReflection()->resources)
	{
		MaterialInstanceResource instanceResource(context);
		instanceResource.name = resource.name;
		instanceResource.kind = resource.kind;
		instanceResource.binding = resource.binding;
		instanceResource.set = resource.set;

		instanceResource.associatedResource = &resource;

		instanceResource.fields.reserve(resource.field.fields.size());

		instanceResource.CollectFields(resource.field, "", instanceResource.fields);

		instanceResource.Repack();

		newResources.push_back(std::move(instanceResource));
	}

	return newResources;
}

void cp::MaterialInstance::ValidateData()
{
	if (!material || !material->GetShaderReflection())
	{
		LOG_ERROR("Material or shader reflection is null");
		return;
	}

	auto correctResources = CreateMaterialInstanceResources();

	for (auto& correctRes : correctResources)
	{
		auto it = std::find_if(resources.begin(), resources.end(), [&correctRes](const MaterialInstanceResource& res) {
			return res.name.compare(correctRes.name) == 0 && res.set == correctRes.set && res.binding == correctRes.binding; // Compare by name, set, and binding
			});
		
		if (it == resources.end()) // If the resource is not found, we simply add it
		{
			resources.push_back(correctRes);
			continue;
		}

		std::vector<MaterialInstanceField> validatedFields;

		for (const auto& correctField : correctRes.fields)
		{
			MaterialInstanceField validatedField = correctField; // Copy the correct field

			auto fieldIt = std::find_if(it->fields.begin(), it->fields.end(), [&validatedField](const MaterialInstanceField& f) {
				return f.name == validatedField.name;
				});

			if (fieldIt != it->fields.end() && fieldIt->data.size() == validatedField.data.size())
			{
				validatedField.data.resize(fieldIt->data.size()); // Resize to match the existing data size
				std::memcpy(validatedField.data.data(), fieldIt->data.data(), fieldIt->data.size() * sizeof(uint8_t)); // Copy the existing data
			}

			validatedFields.push_back(validatedField);
		}

		it->fields = validatedFields; // Update the fields with validated ones
		it->kind = correctRes.kind; // Update the kind in case it was changed
		it->associatedResource = correctRes.associatedResource; // Update the associated resource pointer

		for (const auto& field : it->fields)
		{
			std::string dataStr(field.data.begin(), field.data.end());
		}

		it->Repack(); // Repack the data after validation
	}

	// Removing stale resources that are not in the correctResources

	resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const MaterialInstanceResource& res) {
		return material->GetShaderReflection()->resources.end() == std::find_if(material->GetShaderReflection()->resources.begin(), material->GetShaderReflection()->resources.end(),
			[&res](const ShaderResource& correctRes) {
				return res.name == correctRes.name && res.set == correctRes.set && res.binding == correctRes.binding;
			});
		}), resources.end());
}

#ifdef IN_EDITOR
QWidget* cp::MaterialInstance::CreateMaterialInstanceWidget(QWidget* _parent)
{
	QWidget* widget = new QWidget(_parent);
	QVBoxLayout* layout = new QVBoxLayout(widget);

	for (auto& resource : resources)
	{
		QGroupBox* resourceGroup = new QGroupBox(QString::fromStdString(resource.name), widget);
		QVBoxLayout* resourceLayout = new QVBoxLayout(resourceGroup);

		if (resource.kind == cp::ShaderResourceKind::ConstantBuffer)
		{
			for (const auto& field : resource.fields)
			{
				if (!field.associatedField) continue;
				void* dataPtr = field.GetDataPtr();
				QWidget* fieldWidget = Helper::Material::CreateMaterialFieldWidget(widget, *field.associatedField, dataPtr);
				if(fieldWidget) resourceLayout->addWidget(fieldWidget);
			}
		}

		if (resource.kind == cp::ShaderResourceKind::TextureResource || resource.kind == cp::ShaderResourceKind::Sampler || resource.kind == cp::ShaderResourceKind::CombinedImageSampler)
		{
			cp::TextureDropLineEdit* textureWidget = new cp::TextureDropLineEdit(false, widget);
			textureWidget->SetResourcePath(resource.associatedTexture);
			textureWidget->SetResourcePathOutput(&resource.associatedTexture);
			resourceLayout->addWidget(textureWidget);
		}

		if (resourceLayout->count() > 0)
		{
			layout->addWidget(resourceGroup);
		}
		else
		{
			delete resourceGroup; // If no fields, delete the group
		}
	}

	widget->setLayout(layout);
	return widget;
}
#endif

void cp::MaterialInstanceField::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Name", name);
	_serializer.WriteByteArray("Data", data.size(), data.data());
}

void cp::MaterialInstanceField::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Name", "Error");
	auto [elements, dataRV] = _serializer.ReadByteArray("Data");
	data.resize(elements);
	std::memcpy(data.data(), dataRV, elements * sizeof(uint8_t));
}

cp::MaterialInstanceResource::~MaterialInstanceResource()
{
	if (packedBuffer.buffer)
	{
		Helper::Memory::DestroyBuffer(context->GetDevice(), packedBuffer); // Destroy the packed buffer if it exists
	}
}

void cp::MaterialInstanceResource::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Name", name);
	_serializer.WriteInt("Binding", binding);
	_serializer.WriteInt("Set", set);
	_serializer.WriteInt("Kind", static_cast<int>(kind));

	if (kind == cp::ShaderResourceKind::ConstantBuffer || kind == cp::ShaderResourceKind::StructuredBuffer)
	{

		_serializer.BeginObjectArrayWriting("Fields");
		for (const auto& field : fields)
		{
			_serializer.BeginObjectArrayElementWriting();
			field.Serialize(_serializer);
			_serializer.EndObjectArrayElement();
		}
		_serializer.EndObjectArray();
	}

	if (kind == cp::ShaderResourceKind::TextureResource || kind == cp::ShaderResourceKind::Sampler || kind == cp::ShaderResourceKind::CombinedImageSampler)
	{
		_serializer.WriteString("Associated Texture", associatedTexture);
	}
}

void cp::MaterialInstanceResource::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Name", "Error");
	binding = _serializer.ReadInt("Binding", 0);
	set = _serializer.ReadInt("Set", 0);
	kind = static_cast<cp::ShaderResourceKind>(_serializer.ReadInt("Kind", static_cast<int>(cp::ShaderResourceKind::Unknown)));

	if (kind == cp::ShaderResourceKind::ConstantBuffer || kind == cp::ShaderResourceKind::StructuredBuffer)
	{
		size_t elements = _serializer.BeginObjectArrayReading("Fields");
		for (uint64_t i = 0; i < elements; i++)
		{
			_serializer.BeginObjectArrayElementReading(i);
			cp::MaterialInstanceField field;
			field.Deserialize(_serializer);
			fields.push_back(field);
			_serializer.EndObjectArrayElement();
		}
		_serializer.EndObjectArray();
	}
	
	if (kind == cp::ShaderResourceKind::TextureResource || kind == cp::ShaderResourceKind::Sampler || kind == cp::ShaderResourceKind::CombinedImageSampler)
	{
		associatedTexture = _serializer.ReadString("Associated Texture", "");
	}
}

void cp::MaterialInstanceResource::CollectFields(const ShaderField& field, const std::string& prefix, std::vector<MaterialInstanceField>& fields) const
{
	std::string fullName = prefix.empty() ? field.name : prefix + "." + field.name;

	if (field.fields.empty())
	{
		MaterialInstanceField instanceField;
		instanceField.name = fullName;
		instanceField.data.resize(field.size);
		instanceField.associatedField = &field;
		fields.push_back(instanceField);
	}
	else
	{
		for (const auto& subField : field.fields)
		{
			CollectFields(subField, fullName, fields);
		}
	}
}

void cp::MaterialInstanceResource::Repack()
{
	if (!associatedResource) return;
	if (associatedResource->kind != cp::ShaderResourceKind::ConstantBuffer) return;

	packedData.resize(associatedResource->field.size);

	for (const auto& field : fields)
	{
		if (!field.associatedField) continue;

		size_t offset = field.associatedField->offset;
		if (offset + field.data.size() > packedData.size())
		{
			LOG_ERROR(MF("Field ", field.name, " exceeds packed data size"));
			continue;
		}

		std::memcpy(packedData.data() + offset, field.data.data(), field.data.size());
	}

	if (packedBuffer.buffer)
	{
		Helper::Memory::MapMemory(context->GetDevice(), packedBuffer.memory, packedData.size(), packedData.data()); // Map the memory to the packed data
	}
}

void cp::MaterialInstanceResource::InitBuffer()
{
	if (packedBuffer.buffer)
	{
		Helper::Memory::DestroyBuffer(context->GetDevice(), packedBuffer); // Destroy the packed buffer if it exists
	}

	packedBuffer = Helper::Memory::CreateBuffer(context->GetDevice(), context->GetPhysicalDevice(), packedData.size(), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	Helper::Memory::MapMemory(context->GetDevice(), packedBuffer.memory, packedData.size(), packedData.data()); // Map the memory to the packed data
}

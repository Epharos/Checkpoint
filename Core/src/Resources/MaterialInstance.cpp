#include "pch.hpp"
#include "MaterialInstance.hpp"

#include "Material.hpp"

#include "Util/Serializers/JsonSerializer.hpp"
#include "ResourceManager.hpp"

cp::MaterialInstance::MaterialInstance(const cp::VulkanContext* _context)
{
	context = _context;
}

cp::MaterialInstance::~MaterialInstance()
{
	/*for (auto& [name, desc] : descriptorSets)
	{
		context->GetDescriptorSetManager()->DestroyOrphanedDescriptorSet(desc.descriptorSet);
	}*/
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
		MaterialInstanceResource resource;
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
		MaterialInstanceResource instanceResource;
		instanceResource.name = resource.name;
		instanceResource.kind = resource.kind;
		instanceResource.binding = resource.binding;
		instanceResource.set = resource.set;

		instanceResource.associatedResource = &resource;

		instanceResource.fields.reserve(resource.field.fields.size());

		instanceResource.CollectFields(resource.field, "", instanceResource.fields);

		//instanceResource.packedData.resize(resource.field.size);
		instanceResource.Repack();

		newResources.push_back(std::move(instanceResource));
	}

	return newResources;
}

void cp::MaterialInstance::ValidateData()
{
	//TODO : Make sure the descriptor sets are synchronized with the material

	if (!material || !material->GetShaderReflection())
	{
		LOG_ERROR("Material or shader reflection is null");
		return;
	}

	LOG_DEBUG(MF("Validating material instance data for material: ", material->GetName()));

	auto correctResources = CreateMaterialInstanceResources();

	for (auto& correctRes : correctResources)
	{
		LOG_DEBUG(MF("Comparing ", correctRes.name, " with existing resources"));

		auto it = std::find_if(resources.begin(), resources.end(), [&correctRes](const MaterialInstanceResource& res) {
			return res.name.compare(correctRes.name) == 0 && res.set == correctRes.set && res.binding == correctRes.binding; // Compare by name, set, and binding
			});
		
		if (it == resources.end()) // If the resource is not found, we simply add it
		{
			//LOG_DEBUG(MF("Resource ", correctRes.name, " not found, adding it"));
			resources.push_back(correctRes);
			continue;
		}

		std::vector<MaterialInstanceField> validatedFields;

		//LOG_DEBUG(MF("Resource ", correctRes.name, " found, validating fields"));

		for (const auto& correctField : correctRes.fields)
		{
			MaterialInstanceField validatedField = correctField; // Copy the correct field

			LOG_DEBUG(MF("Validating field ", validatedField.name, " in resource ", correctRes.name));

			auto fieldIt = std::find_if(it->fields.begin(), it->fields.end(), [&validatedField](const MaterialInstanceField& f) {
				return f.name == validatedField.name;
				});

			if (fieldIt != it->fields.end() && fieldIt->data.size() == validatedField.data.size())
			{
				//LOG_DEBUG(MF("Field ", correctField.name, " found in resource ", correctRes.name, ", using existing data"));
				validatedField.data.resize(fieldIt->data.size()); // Resize to match the existing data size
				std::memcpy(validatedField.data.data(), fieldIt->data.data(), fieldIt->data.size() * sizeof(uint8_t)); // Copy the existing data
			}

			validatedFields.push_back(validatedField);
		}

		it->fields = validatedFields; // Update the fields with validated ones
		it->kind = correctRes.kind; // Update the kind in case it was changed
		it->associatedResource = correctRes.associatedResource; // Update the associated resource pointer

		LOG_DEBUG(MF("Resource ", it->name, " validated with ", it->fields.size(), " fields."));
		for (const auto& field : it->fields)
		{
			std::string dataStr(field.data.begin(), field.data.end());
			LOG_DEBUG(MF(" - Field: ", field.name, ", Size: ", field.data.size(), ", Content: ", dataStr));
		}

		//LOG_DEBUG(MF("Repacking resource ", it->name, " after validation"));

		it->Repack(); // Repack the data after validation

		//LOG_DEBUG(MF("Resource ", it->name, " validated and repacked successfully"));
		LOG_DEBUG("--------------------");
	}

	// Removing stale resources that are not in the correctResources

	LOG_DEBUG(MF("Validating resources, removing stale ones..."));

	resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const MaterialInstanceResource& res) {
		return material->GetShaderReflection()->resources.end() == std::find_if(material->GetShaderReflection()->resources.begin(), material->GetShaderReflection()->resources.end(),
			[&res](const ShaderResource& correctRes) {
				return res.name == correctRes.name && res.set == correctRes.set && res.binding == correctRes.binding;
			});
		}), resources.end());

	LOG_DEBUG(MF("Validation complete, ", resources.size(), " resources remaining after validation."));
}

QWidget* cp::MaterialInstance::CreateMaterialInstanceWidget(QWidget* _parent)
{
	QWidget* widget = new QWidget(_parent);
	QVBoxLayout* layout = new QVBoxLayout(widget);

	for (const auto& resource : resources)
	{
		QGroupBox* resourceGroup = new QGroupBox(QString::fromStdString(resource.name), widget);
		QVBoxLayout* resourceLayout = new QVBoxLayout(resourceGroup);

		for (const auto& field : resource.fields)
		{
			if (!field.associatedField) continue;
			void* dataPtr = field.GetDataPtr();
			if (!dataPtr) continue;
			QWidget* fieldWidget = Helper::Material::CreateMaterialFieldWidget(widget, *field.associatedField, dataPtr);
			resourceLayout->addWidget(fieldWidget);
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

void cp::MaterialInstanceResource::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Name", name);
	_serializer.WriteInt("Binding", binding);
	_serializer.WriteInt("Set", set);
	_serializer.WriteInt("Kind", static_cast<int>(kind));

	_serializer.BeginObjectArrayWriting("Fields");
	for (const auto& field : fields)
	{
		_serializer.BeginObjectArrayElementWriting();
		field.Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}
	_serializer.EndObjectArray();
}

void cp::MaterialInstanceResource::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Name", "Error");
	binding = _serializer.ReadInt("Binding", 0);
	set = _serializer.ReadInt("Set", 0);
	kind = static_cast<cp::ShaderResourceKind>(_serializer.ReadInt("Kind", static_cast<int>(cp::ShaderResourceKind::Unknown)));

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

		LOG_DEBUG(MF("Packing ", field.data.size(), " bytes at offset ", offset, " for field ", field.name, " (packedData is used up to byte ", (offset + field.data.size()), "/", packedData.size(), ")"));

		std::memcpy(packedData.data() + offset, field.data.data(), field.data.size() * sizeof(uint8_t));
	}
}

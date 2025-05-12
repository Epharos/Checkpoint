#include "pch.hpp"
#include "MaterialInstance.hpp"

#include "Material.hpp"

#include "Util/Serializers/JsonSerializer.hpp"
#include "ResourceManager.hpp"

cp::MaterialInstance::MaterialInstance(const cp::VulkanContext*& _context)
{
	context = _context;
}

cp::MaterialInstance::~MaterialInstance()
{
	for (auto& [name, desc] : descriptorSets)
	{
		context->GetDescriptorSetManager()->DestroyOrphanedDescriptorSet(desc.descriptorSet);
	}
}

void cp::MaterialInstance::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Associated Material", associatedMaterial);

	if (associatedMaterial.empty())
	{
		LOG_ERROR("MaterialInstance: Associated material is empty");
		return;
	}

	_serializer.BeginObjectArrayWriting("Descriptors");
	for (const auto& [name, desc] : descriptorSets)
	{
		_serializer.BeginObjectArrayElementWriting();
		desc.Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}
	_serializer.EndObjectArray();
}

void cp::MaterialInstance::Deserialize(ISerializer& _serializer)
{
	associatedMaterial = _serializer.ReadString("Associated Material", "");

	if (associatedMaterial.empty())
	{
		LOG_ERROR("MaterialInstance: Associated material is empty");
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

	size_t elements = _serializer.BeginObjectArrayReading("Descriptors");
	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		MaterialInstanceDescriptor desc;
		desc.Deserialize(_serializer);
		auto it = material->GetDescriptors().find(desc.name);

		if (it == material->GetDescriptors().end())
		{
			LOG_ERROR(MF("MaterialInstance: Descriptor ", desc.name, " not found in material ", material->GetName()));
			continue;
		}

		desc.descriptorSet = context->GetDescriptorSetManager()->CreateOrphanedDescriptorSet(it->second.layout);
		descriptorSets.insert({ desc.name, desc });
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();

	SynchronizeDescriptorSetsWithMaterial();
}

void cp::MaterialInstance::SynchronizeDescriptorSetsWithMaterial()
{
	for (const auto& [name, desc] : material->GetDescriptors())
	{
		if (descriptorSets.find(name) == descriptorSets.end())
		{
			MaterialInstanceDescriptor newDesc;
			newDesc.name = name;
			newDesc.descriptorSet = context->GetDescriptorSetManager()->CreateOrphanedDescriptorSet(desc.layout);
			descriptorSets.insert({ name, newDesc });
			newDesc.SynchronizeBindingsWithMaterial(*material);
		}
	}

	for (auto& [name, desc] : descriptorSets)
	{
		auto it = material->GetDescriptors().find(name);
		if(it != material->GetDescriptors().end()) descriptorSets.erase(name);
	}
}

void cp::MaterialInstanceDescriptor::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Name", name);
	_serializer.BeginObjectArrayWriting("Bindings");

	for (const auto& [name, binding] : bindings)
	{
		_serializer.BeginObjectArrayElementWriting();
		binding.Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}

	_serializer.EndObjectArray();
}

void cp::MaterialInstanceDescriptor::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Name", "Error");
	size_t elements = _serializer.BeginObjectArrayReading("Bindings");
	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		MaterialInstanceBinding binding;
		binding.Deserialize(_serializer);
		bindings.insert({ binding.name, binding });
		_serializer.EndObjectArrayElement();
	}
	_serializer.EndObjectArray();
}

cp::MaterialDescriptor& cp::MaterialInstanceDescriptor::GetAssociatedDescriptor(const Material& _material)
{
	return _material.GetDescriptors().at(name);
}

void cp::MaterialInstanceDescriptor::SynchronizeBindingsWithMaterial(const Material& _material)
{
	cp::MaterialDescriptor& desc = GetAssociatedDescriptor(_material);

	for (const auto& [name, binding] : desc.bindings)
	{
		if (bindings.find(name) == bindings.end())
		{
			MaterialInstanceBinding newBinding;
			newBinding.name = name;
			bindings.insert({ name, newBinding });
			newBinding.SynchronizeFieldsWithMaterial(desc);
		}
	}

	for (auto& [name, binding] : bindings)
	{
		auto it = desc.bindings.find(name);

		if (it == desc.bindings.end()) bindings.erase(name);
	}
}

void cp::MaterialInstanceBinding::Serialize(ISerializer& _serializer) const
{
	_serializer.WriteString("Name", name);
	_serializer.BeginObjectArrayWriting("Fields");
	for (const auto& [name, field] : fields)
	{
		_serializer.BeginObjectArrayElementWriting();
		field.Serialize(_serializer);
		_serializer.EndObjectArrayElement();
	}
	_serializer.EndObjectArray();
}

void cp::MaterialInstanceBinding::Deserialize(ISerializer& _serializer)
{
	name = _serializer.ReadString("Name", "Error");
	size_t elements = _serializer.BeginObjectArrayReading("Fields");
	for (uint64_t i = 0; i < elements; i++)
	{
		_serializer.BeginObjectArrayElementReading(i);
		MaterialInstanceField field;
		field.Deserialize(_serializer);
		fields.insert({ field.name, field });
		_serializer.EndObjectArrayElement();
	}
	_serializer.EndObjectArray();
}

cp::MaterialBinding& cp::MaterialInstanceBinding::GetAssociatedBinding(const MaterialDescriptor& _material)
{
	return _material.GetBindings().at(name);
}

void cp::MaterialInstanceBinding::SynchronizeFieldsWithMaterial(const MaterialDescriptor& _material)
{
	cp::MaterialBinding& binding = GetAssociatedBinding(_material);

	for (const auto& [name, field] : binding.fields)
	{
		if (fields.find(name) == fields.end())
		{
			MaterialInstanceField newField;
			newField.name = name;
			fields.insert({ name, newField });
		}
	}
	for (auto& [name, field] : fields)
	{
		auto it = binding.fields.find(name);
		if (it == binding.fields.end()) fields.erase(name);
	}
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
	data.reserve(elements);
	
	for (size_t i = 0; i < elements; i++)
	{
		data[i] = dataRV[i];
	}
}

cp::MaterialField& cp::MaterialInstanceField::GetAssociatedField(MaterialBinding& _material)
{
	return _material.fields.at(name);
}

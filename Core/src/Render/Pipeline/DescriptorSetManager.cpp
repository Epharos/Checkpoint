#include "pch.hpp"
#include "DescriptorSetManager.hpp"

cp::DescriptorSetManager::DescriptorSetManager(vk::Device _device) : device(_device)
{
	vk::DescriptorPoolSize poolSize = {};
	poolSize.type = vk::DescriptorType::eUniformBuffer;
	poolSize.descriptorCount = 100;

	vk::DescriptorPoolCreateInfo poolInfo = {};
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 100;
	poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

	pool = device.createDescriptorPool(poolInfo);
}

vk::DescriptorSet& cp::DescriptorSetManager::GetDescriptorSet(const std::string& _name)
{
	return sets[_name];
}

void cp::DescriptorSetManager::UpdateDescriptorSet(const std::string& _name, const DescriptorSetUpdate& _write)
{
#ifdef _DEBUG
	if (sets.find(_name) == sets.end())
		throw std::runtime_error("Descriptor set with name " + _name + " does not exist");
#endif

	vk::WriteDescriptorSet write = {};
	write.dstSet = sets[_name];
	write.dstBinding = _write.dstBinding;
	write.dstArrayElement = _write.dstArrayElement;
	write.descriptorType = _write.descriptorType;
	write.descriptorCount = _write.descriptorCount;

	switch (_write.updateType)
	{
	case DescriptorSetUpdateType::BUFFER:
		write.pBufferInfo = new vk::DescriptorBufferInfo(_write.buffer, _write.offset, _write.range);
		break;
	case DescriptorSetUpdateType::IMAGE:
		write.pImageInfo = new vk::DescriptorImageInfo(_write.sampler, _write.imageView, _write.imageLayout);
		break;
	}

	device.updateDescriptorSets(1, &write, 0, nullptr);
}

void cp::DescriptorSetManager::UpdateDescriptorSet(const std::string& _name, const std::vector<DescriptorSetUpdate>& _writes)
{
#ifdef _DEBUG
	if (sets.find(_name) == sets.end())
		throw std::runtime_error("Descriptor set with name " + _name + " does not exist");
#endif

	std::vector<vk::WriteDescriptorSet> writes;

	for (const auto& writeIndex : _writes)
	{
		vk::WriteDescriptorSet write = {};
		write.dstSet = sets[_name];
		write.dstBinding = writeIndex.dstBinding;
		write.dstArrayElement = writeIndex.dstArrayElement;
		write.descriptorType = writeIndex.descriptorType;
		write.descriptorCount = writeIndex.descriptorCount;

		switch (writeIndex.updateType)
		{
		case DescriptorSetUpdateType::BUFFER:
			write.pBufferInfo = new vk::DescriptorBufferInfo(writeIndex.buffer, writeIndex.offset, writeIndex.range);
			break;
		case DescriptorSetUpdateType::IMAGE:
			write.pImageInfo = new vk::DescriptorImageInfo(writeIndex.sampler, writeIndex.imageView, writeIndex.imageLayout);
			break;
		}

		writes.push_back(write);
	}

	device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void cp::DescriptorSetManager::UpdateDescriptorSets(const std::vector<std::string>& _names, const std::vector<DescriptorSetUpdate>& _writes)
{
#ifdef _DEBUG
	if (_names.size() != _writes.size())
		throw std::runtime_error("Names and update data sizes do not match");

	for (const auto& name : _names)
		if (sets.find(name) == sets.end())
			throw std::runtime_error("Descriptor set with name " + name + " does not exist");
#endif

	std::vector<vk::WriteDescriptorSet> writes;

	for (size_t i = 0; i < _names.size(); i++)
	{
		vk::WriteDescriptorSet write = {};
		write.dstSet = sets[_names[i]];
		write.dstBinding = _writes[i].dstBinding;
		write.dstArrayElement = _writes[i].dstArrayElement;
		write.descriptorType = _writes[i].descriptorType;
		write.descriptorCount = _writes[i].descriptorCount;

		switch (_writes[i].updateType)
		{
		case DescriptorSetUpdateType::BUFFER:
			write.pBufferInfo = new vk::DescriptorBufferInfo(_writes[i].buffer, _writes[i].offset, _writes[i].range);
			break;
		case DescriptorSetUpdateType::IMAGE:
			write.pImageInfo = new vk::DescriptorImageInfo(_writes[i].sampler, _writes[i].imageView, _writes[i].imageLayout);
			break;
		}

		writes.push_back(write);
	}

	device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

vk::DescriptorSet cp::DescriptorSetManager::CreateDescriptorSet(const std::string& _name, const vk::DescriptorSetLayout& _layout)
{
#ifdef _DEBUG
	if (sets.find(_name) != sets.end())
		throw std::runtime_error("Descriptor set with name " + _name + " already exists");
#endif

	sets[_name] = CreateOrphanedDescriptorSet(_layout);
	return sets[_name];
}

std::vector<vk::DescriptorSet> cp::DescriptorSetManager::CreateDescriptorSets(const std::vector<std::string>& _names, const std::vector<vk::DescriptorSetLayout>& _layouts)
{
#ifdef _DEBUG
	if (_names.size() != _layouts.size())
		throw std::runtime_error("Names and layouts sizes do not match");

	for (const auto& name : _names)
		if (sets.find(name) != sets.end())
			throw std::runtime_error("Descriptor set with name " + name + " already exists");
#endif

	std::vector<vk::DescriptorSet> results = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(pool, static_cast<uint32_t>(_names.size()), _layouts.data()));

	for (size_t i = 0; i < _names.size(); i++)
		sets[_names[i]] = results[i];

	return results;
}

vk::DescriptorSet cp::DescriptorSetManager::CreateOrphanedDescriptorSet(const vk::DescriptorSetLayout& _layout)
{
	return device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(pool, 1, &_layout))[0];
}

void cp::DescriptorSetManager::UpdateOrphanedDescriptorSet(const vk::DescriptorSet& _set, const DescriptorSetUpdate& _write)
{
	vk::WriteDescriptorSet write = {};
	write.dstSet = _set;
	write.dstBinding = _write.dstBinding;
	write.dstArrayElement = _write.dstArrayElement;
	write.descriptorType = _write.descriptorType;
	write.descriptorCount = _write.descriptorCount;

	switch (_write.updateType)
	{
	case DescriptorSetUpdateType::BUFFER:
		write.pBufferInfo = new vk::DescriptorBufferInfo(_write.buffer, _write.offset, _write.range);
		break;
	case DescriptorSetUpdateType::IMAGE:
		write.pImageInfo = new vk::DescriptorImageInfo(_write.sampler, _write.imageView, _write.imageLayout);
		break;
	}

	device.updateDescriptorSets(1, &write, 0, nullptr);
}

void cp::DescriptorSetManager::UpdateOrphanedDescriptorSet(const vk::DescriptorSet& _set, const std::vector<DescriptorSetUpdate>& _writes)
{
	std::vector<vk::WriteDescriptorSet> writes;

	for (const auto& writeIndex : _writes)
	{
		vk::WriteDescriptorSet write = {};
		write.dstSet = _set;
		write.dstBinding = writeIndex.dstBinding;
		write.dstArrayElement = writeIndex.dstArrayElement;
		write.descriptorType = writeIndex.descriptorType;
		write.descriptorCount = writeIndex.descriptorCount;

		switch (writeIndex.updateType)
		{
		case DescriptorSetUpdateType::BUFFER:
			write.pBufferInfo = new vk::DescriptorBufferInfo(writeIndex.buffer, writeIndex.offset, writeIndex.range);
			break;
		case DescriptorSetUpdateType::IMAGE:
			write.pImageInfo = new vk::DescriptorImageInfo(writeIndex.sampler, writeIndex.imageView, writeIndex.imageLayout);
			break;
		}

		writes.push_back(write);
	}

	device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

}

void cp::DescriptorSetManager::DestroyOrphanedDescriptorSet(const vk::DescriptorSet& _set)
{
	device.freeDescriptorSets(pool, _set);
}

void cp::DescriptorSetManager::DestroyDescriptorSet(const std::string& _name)
{
#ifdef _DEBUG
	if (sets.find(_name) == sets.end())
		throw std::runtime_error("Descriptor set with name " + _name + " does not exist");
#endif

	device.freeDescriptorSets(pool, sets[_name]);
	sets.erase(_name);
}

void cp::DescriptorSetManager::Cleanup()
{
	for (auto& set : sets)
		device.freeDescriptorSets(pool, set.second);

	device.destroyDescriptorPool(pool);
}

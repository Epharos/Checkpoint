#include "pch.hpp"

#include "SetLayoutsManager.hpp"

Pipeline::DescriptorSetLayoutsManager::DescriptorSetLayoutsManager(vk::Device _device) : device(_device)
{

}

vk::DescriptorSetLayout Pipeline::DescriptorSetLayoutsManager::GetDescriptorSetLayout(const std::string& _name)
{
	return layouts.at(_name);
}

vk::DescriptorSetLayout Pipeline::DescriptorSetLayoutsManager::CreateDescriptorSetLayout(const std::string& _name, const std::vector<vk::DescriptorSetLayoutBinding>& _bindings)
{
	if (layouts.find(_name) == layouts.end())
	{
		LOG_TRACE(MF("Creating new descriptor set layout [", _name, "]"));

		vk::DescriptorSetLayoutCreateInfo createInfo = {
			{},
			static_cast<std::uint32_t>(_bindings.size()),
			_bindings.data()
		};

		layouts[_name] = device.createDescriptorSetLayout(createInfo);
	}

	return layouts[_name];
}

void Pipeline::DescriptorSetLayoutsManager::Cleanup()
{
	for (auto& layout : layouts)
	{
		device.destroyDescriptorSetLayout(layout.second);
	}
}


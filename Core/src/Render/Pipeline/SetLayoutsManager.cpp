#include "pch.hpp"

#include "SetLayoutsManager.hpp"

cp::DescriptorSetLayoutsManager::DescriptorSetLayoutsManager(vk::Device _device) : device(_device)
{

}

vk::DescriptorSetLayout cp::DescriptorSetLayoutsManager::GetDescriptorSetLayout(const std::string& _name)
{
	return layouts.at(_name);
}

vk::DescriptorSetLayout cp::DescriptorSetLayoutsManager::CreateDescriptorSetLayout(const std::string& _name, const std::vector<vk::DescriptorSetLayoutBinding>& _bindings)
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

void cp::DescriptorSetLayoutsManager::Cleanup()
{
	for (auto& layout : layouts)
	{
		device.destroyDescriptorSetLayout(layout.second);
	}
}


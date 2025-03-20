#include "pch.hpp"

#include "SetLayoutsManager.hpp"

cp::DescriptorSetLayoutsManager::DescriptorSetLayoutsManager(vk::Device _device) : device(_device)
{
	// Initialize global descriptor set layout holding camera data for lit shader
	globalLit = CreateDescriptorSetLayout("Global Lit", {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex), //Camera data
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment), //Sun (directional light) data
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex), //Cascades
		vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eSampledImage, 1, vk::ShaderStageFlagBits::eFragment), //Shadow map texture array
		});

	globalUnlit = CreateDescriptorSetLayout("Global Unlit", {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex), //Camera data
		});

	instancedDrawing = CreateDescriptorSetLayout("Instanced Drawing", {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex), //Model instances
		});
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

vk::DescriptorSetLayout cp::DescriptorSetLayoutsManager::OverrideDescriptorSetLayout(const std::string& _name, const std::vector<vk::DescriptorSetLayoutBinding>& _bindings)
{
	if (layouts.find(_name) == layouts.end())
	{
		LOG_ERROR("Couldn't override descriptor set layout");
		return VK_NULL_HANDLE;
	}

	device.destroyDescriptorSetLayout(layouts.at(_name));

	vk::DescriptorSetLayoutCreateInfo createInfo = {
			{},
			static_cast<std::uint32_t>(_bindings.size()),
			_bindings.data()
	};

	layouts[_name] = device.createDescriptorSetLayout(createInfo);
}

void cp::DescriptorSetLayoutsManager::DestroyDescriptorSetLayout(const std::string& _name)
{
	if (layouts.find(_name) != layouts.end())
	{
		device.destroyDescriptorSetLayout(layouts[_name]);
		layouts.erase(_name);
		return;
	}

	LOG_WARNING(MF("Could not destroy descriptor set layout (", _name, ")"));
}

void cp::DescriptorSetLayoutsManager::DestroyDescriptorSetLayout(const vk::DescriptorSetLayout& _layout)
{
	for (auto& layout : layouts)
	{
		if (layout.second == _layout)
		{
			device.destroyDescriptorSetLayout(layout.second);
			layouts.erase(layout.first);
			return;
		}
	}

	LOG_WARNING(MF("Could not destroy descriptor set layout handle (", _layout, ")"));
}

void cp::DescriptorSetLayoutsManager::Cleanup()
{
	for (auto& layout : layouts)
	{
		device.destroyDescriptorSetLayout(layout.second);
	}
}


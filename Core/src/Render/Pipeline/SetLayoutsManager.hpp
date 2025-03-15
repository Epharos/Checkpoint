#pragma once

#include "../../pch.hpp"

namespace cp
{
	class DescriptorSetLayoutsManager
	{
	private:
		vk::Device device;

		std::unordered_map<std::string, vk::DescriptorSetLayout> layouts;

	public:
		DescriptorSetLayoutsManager(vk::Device _device);

		vk::DescriptorSetLayout GetDescriptorSetLayout(const std::string& _name);
		vk::DescriptorSetLayout CreateDescriptorSetLayout(const std::string& _name, const std::vector<vk::DescriptorSetLayoutBinding>& _bindings);


		void Cleanup();
	};
}
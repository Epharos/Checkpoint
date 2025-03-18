#pragma once

#include "../../pch.hpp"

namespace cp
{
	class DescriptorSetLayoutsManager
	{
	private:
		vk::Device device;

		std::unordered_map<std::string, vk::DescriptorSetLayout> layouts;

		vk::DescriptorSetLayout globalLit;
		vk::DescriptorSetLayout globalUnlit;
		vk::DescriptorSetLayout instancedDrawing;

	public:
		DescriptorSetLayoutsManager(vk::Device _device);

		vk::DescriptorSetLayout GetDescriptorSetLayout(const std::string& _name);
		vk::DescriptorSetLayout CreateDescriptorSetLayout(const std::string& _name, const std::vector<vk::DescriptorSetLayoutBinding>& _bindings);

		inline constexpr vk::DescriptorSetLayout GlobalLit() { return globalLit; }
		inline constexpr vk::DescriptorSetLayout GlobalUnlit() { return globalUnlit; }
		inline constexpr vk::DescriptorSetLayout InstancedDrawing() { return instancedDrawing; }

		void Cleanup();
	};
}
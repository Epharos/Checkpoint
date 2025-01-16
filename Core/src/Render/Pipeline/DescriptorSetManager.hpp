#pragma once

#include "../../pch.hpp"

namespace Pipeline
{
	struct DescriptorSetUpdate
	{
		vk::Buffer buffer;
		vk::DeviceSize offset;
		vk::DeviceSize range;

		uint32_t dstBinding;
		uint32_t dstArrayElement;
		vk::DescriptorType descriptorType;
		uint32_t descriptorCount;
	};

	class DescriptorSetManager
	{
	private:
		vk::Device device;
		vk::DescriptorPool pool;

		std::unordered_map<std::string, vk::DescriptorSet> sets;

	public:
		DescriptorSetManager(vk::Device _device);

		void Cleanup();

		vk::DescriptorSet& GetDescriptorSet(const std::string& _name);

		void UpdateDescriptorSet(const std::string& _name, const DescriptorSetUpdate& _write);
		void UpdateDescriptorSets(const std::vector<std::string>& _names, const std::vector<DescriptorSetUpdate>& _writes);

		vk::DescriptorSet CreateDescriptorSet(const std::string& _name, const vk::DescriptorSetLayout& _layout);
		std::vector<vk::DescriptorSet> CreateDescriptorSets(const std::vector<std::string>& _names, const std::vector<vk::DescriptorSetLayout>& _layouts);

		void DestroyDescriptorSet(const std::string& _name);

		inline constexpr const vk::DescriptorPool GetDescriptorPool() const { return pool; }
	};
}

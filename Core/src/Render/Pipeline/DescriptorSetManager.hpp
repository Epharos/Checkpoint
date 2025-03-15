#pragma once

#include "../../pch.hpp"

namespace cp
{
	enum DescriptorSetUpdateType
	{
		BUFFER,
		IMAGE
	};

	struct DescriptorSetUpdate
	{
		DescriptorSetUpdateType updateType = DescriptorSetUpdateType::BUFFER;

		vk::Buffer buffer;
		vk::DeviceSize offset;
		vk::DeviceSize range;

		vk::ImageLayout imageLayout;
		vk::ImageView imageView;
		vk::Sampler sampler;

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
		void UpdateDescriptorSet(const std::string& _name, const std::vector<DescriptorSetUpdate>& _writes);
		void UpdateDescriptorSets(const std::vector<std::string>& _names, const std::vector<DescriptorSetUpdate>& _writes);

		vk::DescriptorSet CreateDescriptorSet(const std::string& _name, const vk::DescriptorSetLayout& _layout);
		std::vector<vk::DescriptorSet> CreateDescriptorSets(const std::vector<std::string>& _names, const std::vector<vk::DescriptorSetLayout>& _layouts);

		vk::DescriptorSet CreateOrphanedDescriptorSet(const vk::DescriptorSetLayout& _layout);
		void UpdateOrphanedDescriptorSet(const vk::DescriptorSet& _set, const DescriptorSetUpdate& _write);
		void UpdateOrphanedDescriptorSet(const vk::DescriptorSet& _set, const std::vector<DescriptorSetUpdate>& _writes);
		void DestroyOrphanedDescriptorSet(const vk::DescriptorSet& _set);

		void DestroyDescriptorSet(const std::string& _name);

		inline constexpr const vk::DescriptorPool GetDescriptorPool() const { return pool; }
	};
}

#pragma once

#include "../pch.hpp"

#include <RHI/Data.hpp>

namespace cp
{
	class Device;

	class DescriptorSetLayout final : public IDescriptorSetLayout
	{
	public:
		DescriptorSetLayout(const DescriptorSetLayoutInfo& _info, Device& _device);
		~DescriptorSetLayout() override;

		[[nodiscard]] vk::DescriptorSetLayout& GetHandle() { return descriptorSetLayout; }
		[[nodiscard]] const vk::DescriptorSetLayout& GetHandle() const { return descriptorSetLayout; }

		[[nodiscard]] const DescriptorSetLayoutInfo& GetInfo() const { return info; }
		[[nodiscard]] const DescriptorBinding* FindBinding(uint32_t _binding) const;

	private:
		void Initialize();
		void Cleanup() const;

	private:
		vk::DescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
		Device& device;
	};
}

#include "../pch.hpp"

#include "DescriptorSetLayout.hpp"

#include "../Core/Device.hpp"
#include "../Utilities/VulkanConverter.hpp"

namespace cp
{
	DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutInfo& _info, Device& _device)
		: IDescriptorSetLayout(_info), device(_device)
	{
		Initialize();
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		Cleanup();
	}

	const DescriptorBinding* DescriptorSetLayout::FindBinding(const uint32_t _binding) const
	{
		for (const DescriptorBinding& binding : info.bindings)
		{
			if (binding.binding == _binding)
			{
				return &binding;
			}
		}

		return nullptr;
	}

	void DescriptorSetLayout::Initialize()
	{
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		bindings.reserve(info.bindings.size());

		for (const auto& [binding, type, count, visibility] : info.bindings)
		{
			vk::DescriptorSetLayoutBinding vkBinding;
			vkBinding.setBinding(binding);
			vkBinding.setDescriptorType(EnumCast<vk::DescriptorType>(type));
			vkBinding.setDescriptorCount(count);
			vkBinding.setStageFlags(EnumBitsCast<vk::ShaderStageFlags>(visibility));
			vkBinding.setPImmutableSamplers(nullptr);

			bindings.push_back(vkBinding);
		}

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		descriptorSetLayoutCreateInfo.setBindingCount(static_cast<uint32_t>(bindings.size()));
		descriptorSetLayoutCreateInfo.setPBindings(bindings.data());

		CP_VK_CHECK(device.GetHandle().createDescriptorSetLayout(&descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));
		CP_ENSURE_MSG(descriptorSetLayout != VK_NULL_HANDLE, "DescriptorSetLayout was not initialized");
	}

	void DescriptorSetLayout::Cleanup() const
	{
		if (descriptorSetLayout != VK_NULL_HANDLE)
		{
			device.GetHandle().destroyDescriptorSetLayout(descriptorSetLayout);
		}
	}
}

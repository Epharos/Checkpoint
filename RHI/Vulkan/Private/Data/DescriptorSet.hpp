#pragma once

#include "../pch.hpp"

#include <RHI/Data.hpp>

namespace cp
{
	class Device;

	class DescriptorSet final : public IDescriptorSet
	{
	public:
		DescriptorSet(const IDescriptorSetLayout& _layout, Device& _device);
		~DescriptorSet() override;

		void UpdateBuffers(const std::vector<DescriptorBufferBinding>& _buffers) override;
		void UpdateTextures(const std::vector<DescriptorTextureBinding>& _textures) override;

		[[nodiscard]] const IDescriptorSetLayout& GetLayout() const override { return descriptorSetLayout; }

		[[nodiscard]] vk::DescriptorSet& GetHandle() { return descriptorSet; }
		[[nodiscard]] const vk::DescriptorSet& GetHandle() const { return descriptorSet; }

	private:
		void Initialize();
		void Cleanup() const;

	private:
		vk::DescriptorPool descriptorPool{ VK_NULL_HANDLE };
		vk::DescriptorSet descriptorSet{ VK_NULL_HANDLE };

		Device& device;
	};
}

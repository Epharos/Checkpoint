#include "../pch.hpp"

#include "DescriptorSet.hpp"

#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"
#include "DescriptorSetLayout.hpp"
#include "../Core/Device.hpp"
#include "../Utilities/VulkanConverter.hpp"

namespace cp
{
	namespace
	{
		void AddPoolSize(std::vector<vk::DescriptorPoolSize>& _poolSizes, const vk::DescriptorType _descriptorType, const uint32_t _count)
		{
			for (vk::DescriptorPoolSize& poolSize : _poolSizes)
			{
				if (poolSize.type == _descriptorType)
				{
					poolSize.descriptorCount += _count;
					return;
				}
			}

			_poolSizes.push_back({ _descriptorType, _count });
		}
	}

	DescriptorSet::DescriptorSet(const IDescriptorSetLayout& _layout, Device& _device)
		: IDescriptorSet(_layout), device(_device)
	{
		Initialize();
	}

	DescriptorSet::~DescriptorSet()
	{
		Cleanup();
	}

	void DescriptorSet::UpdateBuffers(const std::vector<DescriptorBufferBinding>& _buffers)
	{
		const auto& layout = static_cast<const DescriptorSetLayout&>(descriptorSetLayout);

		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		bufferInfos.reserve(_buffers.size());

		std::vector<vk::WriteDescriptorSet> writes;
		writes.reserve(_buffers.size());

		for (const DescriptorBufferBinding& binding : _buffers)
		{
			CP_EXPECT_MSG(binding.buffer, "Descriptor buffer binding contains a null buffer");

			const DescriptorBinding* descriptorBinding = layout.FindBinding(binding.binding);
			CP_EXPECT_MSG(descriptorBinding, "Descriptor binding is not part of descriptor set layout");

			const auto descriptorType = EnumCast<vk::DescriptorType>(descriptorBinding->type);
			CP_EXPECT_MSG(
				descriptorType == vk::DescriptorType::eUniformBuffer || descriptorType == vk::DescriptorType::eStorageBuffer,
				"Descriptor binding type is not a buffer descriptor type"
			);

			const auto& buffer = static_cast<const Buffer&>(*binding.buffer);
			CP_EXPECT_MSG(binding.offsetBytes <= buffer.GetSizeBytes(), "Descriptor buffer offset is out of bounds");
			const auto rangeBytes = binding.rangeBytes == ~0ull ? buffer.GetSizeBytes() - binding.offsetBytes : binding.rangeBytes;
			CP_EXPECT_MSG(rangeBytes > 0, "Descriptor buffer range must be greater than zero");

			vk::DescriptorBufferInfo bufferInfo;
			bufferInfo.setBuffer(buffer.GetHandle());
			bufferInfo.setOffset(binding.offsetBytes);
			bufferInfo.setRange(rangeBytes);

			bufferInfos.push_back(bufferInfo);

			vk::WriteDescriptorSet write;
			write.setDstSet(descriptorSet);
			write.setDstBinding(binding.binding);
			write.setDstArrayElement(0);
			write.setDescriptorCount(1);
			write.setDescriptorType(descriptorType);
			write.setPBufferInfo(&bufferInfos.back());

			writes.push_back(write);
		}

		device.GetHandle().updateDescriptorSets(writes, {});
	}

	void DescriptorSet::UpdateTextures(const std::vector<DescriptorTextureBinding>& _textures)
	{
		const auto& layout = static_cast<const DescriptorSetLayout&>(descriptorSetLayout);

		std::vector<vk::DescriptorImageInfo> imageInfos;
		imageInfos.reserve(_textures.size());

		std::vector<vk::WriteDescriptorSet> writes;
		writes.reserve(_textures.size());

		for (const DescriptorTextureBinding& binding : _textures)
		{
			const DescriptorBinding* descriptorBinding = layout.FindBinding(binding.binding);
			CP_EXPECT_MSG(descriptorBinding, "Descriptor binding is not part of descriptor set layout");

			const auto descriptorType = EnumCast<vk::DescriptorType>(descriptorBinding->type);
			const bool isSamplerType = descriptorType == vk::DescriptorType::eSampler;
			const bool isImageType = descriptorType == vk::DescriptorType::eSampledImage || descriptorType == vk::DescriptorType::eStorageImage;
			const bool isCombinedType = descriptorType == vk::DescriptorType::eCombinedImageSampler;

			CP_EXPECT_MSG(isSamplerType || isImageType || isCombinedType, "Descriptor binding type is not a texture/sampler descriptor type");

			vk::DescriptorImageInfo imageInfo;
			imageInfo.setImageLayout(EnumCast<vk::ImageLayout>(binding.layout));

			if (binding.texture)
			{
				const auto& texture = static_cast<const Texture&>(*binding.texture);
				imageInfo.setImageView(texture.GetImageView());
			}
			else
			{
				CP_EXPECT_MSG(isSamplerType, "Texture descriptor update requires a texture for this binding type");
			}

			if (binding.sampler)
			{
				const auto& sampler = static_cast<const Sampler&>(*binding.sampler);
				imageInfo.setSampler(sampler.GetHandle());
			}
			else
			{
				CP_EXPECT_MSG(!isCombinedType && !isSamplerType, "Sampler descriptor update requires a sampler");
			}

			imageInfos.push_back(imageInfo);

			vk::WriteDescriptorSet write;
			write.setDstSet(descriptorSet);
			write.setDstBinding(binding.binding);
			write.setDstArrayElement(0);
			write.setDescriptorCount(1);
			write.setDescriptorType(descriptorType);
			write.setPImageInfo(&imageInfos.back());

			writes.push_back(write);
		}

		device.GetHandle().updateDescriptorSets(writes, {});
	}

	void DescriptorSet::Initialize()
	{
		const auto& layout = static_cast<const DescriptorSetLayout&>(descriptorSetLayout);

		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const DescriptorBinding& binding : layout.GetInfo().bindings)
		{
			AddPoolSize(poolSizes, EnumCast<vk::DescriptorType>(binding.type), binding.count);
		}

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
		descriptorPoolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()));
		descriptorPoolCreateInfo.setPPoolSizes(poolSizes.data());
		descriptorPoolCreateInfo.setMaxSets(1);

		CP_VK_CHECK(device.GetHandle().createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &descriptorPool));
		CP_ENSURE_MSG(descriptorPool != VK_NULL_HANDLE, "DescriptorPool was not initialized");

		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.setDescriptorPool(descriptorPool);
		descriptorSetAllocateInfo.setDescriptorSetCount(1);
		const auto descriptorSetLayoutHandle = layout.GetHandle();
		descriptorSetAllocateInfo.setPSetLayouts(&descriptorSetLayoutHandle);

		descriptorSet = device.GetHandle().allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
		CP_ENSURE_MSG(descriptorSet != VK_NULL_HANDLE, "DescriptorSet was not initialized");
	}

	void DescriptorSet::Cleanup() const
	{
		if (descriptorPool != VK_NULL_HANDLE)
		{
			device.GetHandle().destroyDescriptorPool(descriptorPool);
		}
	}
}

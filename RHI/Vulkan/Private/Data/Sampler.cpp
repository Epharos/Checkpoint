#include "../pch.hpp"

#include "Sampler.hpp"

#include "../Core/Device.hpp"
#include "../Utilities/VulkanConverter.hpp"

namespace cp
{
	Sampler::Sampler(const SamplerInfo& _info, Device& _device)
		: ISampler(_info), device(_device)
	{
		Initialize();
	}

	Sampler::~Sampler()
	{
		Cleanup();
	}

	void Sampler::Initialize()
	{
		vk::SamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.setMagFilter(EnumCast<vk::Filter>(info.magFilter));
		samplerCreateInfo.setMinFilter(EnumCast<vk::Filter>(info.minFilter));
		samplerCreateInfo.setMipmapMode(EnumCast<vk::SamplerMipmapMode>(info.mipmapMode));
		samplerCreateInfo.setAddressModeU(EnumCast<vk::SamplerAddressMode>(info.addressU));
		samplerCreateInfo.setAddressModeV(EnumCast<vk::SamplerAddressMode>(info.addressV));
		samplerCreateInfo.setAddressModeW(EnumCast<vk::SamplerAddressMode>(info.addressW));
		samplerCreateInfo.setAnisotropyEnable(info.anisotropyEnable);
		samplerCreateInfo.setMaxAnisotropy(info.maxAnisotropy);
		samplerCreateInfo.setMinLod(0.0f);
		samplerCreateInfo.setMaxLod(VK_LOD_CLAMP_NONE);
		samplerCreateInfo.setBorderColor(vk::BorderColor::eFloatOpaqueBlack);
		samplerCreateInfo.setUnnormalizedCoordinates(false);

		CP_VK_CHECK(device.GetHandle().createSampler(&samplerCreateInfo, nullptr, &sampler));
		CP_ENSURE_MSG(sampler != VK_NULL_HANDLE, "Sampler was not initialized");
	}

	void Sampler::Cleanup() const
	{
		if (sampler != VK_NULL_HANDLE)
		{
			device.GetHandle().destroySampler(sampler);
		}
	}
}

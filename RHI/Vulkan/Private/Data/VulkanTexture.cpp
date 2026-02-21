#include "../pch.hpp"

#include "VulkanTexture.hpp"

#include <Log.hpp>

#include "../Core/VulkanDevice.hpp"
#include "../Core/VulkanPhysicalDevice.hpp"
#include "../Utilities/EnumConverter.hpp"
#include "../Utilities/MemoryHelper.hpp"

namespace cp
{
	VulkanTexture::VulkanTexture(ILogger& _logger, const TextureInfo& _info, std::shared_ptr<VulkanDevice> _device)
		: ITexture(_info), device(_device)
	{
		resource = std::make_shared<VulkanTextureResource>(_info, _device);

		_logger.Log(CP_LOG_EVENT(ILogger::Info, VulkanRHI_Label, cp::Message::Create<TextComponent>("Created texture"))); // TODO : Add more information about the texture created

		CP_ENSURE_MSG(resource, "Could not create texture");
		CP_ENSURE_MSG(view != VK_NULL_HANDLE, "View could not be created");
	}

	VulkanTexture::VulkanTexture(ILogger& _logger, vk::Image& _image)
		: ITexture(TextureInfo{}), device(nullptr)
	{
		resource = std::make_shared<VulkanTextureResource>(_image);

		_logger.Log(CP_LOG_EVENT(ILogger::Info, VulkanRHI_Label, cp::Message::Create<TextComponent>("Texture access created")));

		CP_ENSURE_MSG(resource, "Could not store texture");
		CP_ENSURE_MSG(view != VK_NULL_HANDLE, "View could not be created");
	}

	VulkanTexture::~VulkanTexture()
	{
		Cleanup();
	}

	void VulkanTexture::Initiate()
	{
		vk::ImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.setImage(resource->image);
		viewCreateInfo.setFormat(ConvertToVulkanFormat(info.format));

		vk::ImageViewType type = vk::ImageViewType::e2D;

		if(info.extent.z() != 0)
		{
			type = vk::ImageViewType::e3D;
		}
		else if (info.arrayLayers > 1)
		{
			type = vk::ImageViewType::e2DArray;
		}
		else
		{
			type = vk::ImageViewType::e2D;
		}

		viewCreateInfo.setViewType(type);

		vk::ImageSubresourceRange subresourceRange = {};
		//subresourceRange.setAspectMask()
		subresourceRange.setBaseMipLevel(0);
		subresourceRange.setLevelCount(info.mipLevels);
		subresourceRange.setBaseArrayLayer(0);
		subresourceRange.setLayerCount(info.arrayLayers);
		subresourceRange.setAspectMask(ConvertToVulkanImageAspectFlags(info.format, info.usage));

		//viewCreateInfo.setSubresourceRange

		// TODO : Create Image View
	}

	void VulkanTexture::Cleanup()
	{
		CP_EXPECT_MSG(view != VK_NULL_HANDLE, "View shouldn't be null");

		device->GetHandle().destroyImageView(view);
	}

	VulkanTextureResource::VulkanTextureResource(vk::Image& _image) :
		image(_image), memory(VK_NULL_HANDLE), device(nullptr), isOwner(false)
	{

	}

	VulkanTextureResource::VulkanTextureResource(const TextureInfo& _info, std::shared_ptr<VulkanDevice> _device)
		: device(_device)
	{
		CP_EXPECT_MSG(_device, "VulkanDevice cannot be null");

		vk::ImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.setImageType(_info.extent.z() != 0 ? vk::ImageType::e3D : vk::ImageType::e2D);
		imageCreateInfo.setFormat(ConvertToVulkanFormat(_info.format));
		imageCreateInfo.setExtent({ _info.extent.x(), _info.extent.y(), _info.extent.z() });
		imageCreateInfo.setMipLevels(_info.mipLevels);
		imageCreateInfo.setArrayLayers(_info.arrayLayers);
		imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
		imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
		imageCreateInfo.setUsage(ConvertToVulkanImageUsageFlags(_info.usage));
		imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
		imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);

		CP_VK_CHECK(device->GetHandle().createImage(&imageCreateInfo, nullptr, &image));
		CP_ENSURE_MSG(image != VK_NULL_HANDLE, "Image was not initialized");

		vk::MemoryRequirements memoryRequirements = device->GetHandle().getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.setAllocationSize(memoryRequirements.size);
		memoryAllocateInfo.setMemoryTypeIndex(
			FindMemoryType(
				device->GetPhysicalDevice().GetHandle(), 
				memoryRequirements.memoryTypeBits, 
				vk::MemoryPropertyFlagBits::eDeviceLocal
			)
		);

		memory = device->GetHandle().allocateMemory(memoryAllocateInfo);

		CP_ENSURE_MSG(memory != VK_NULL_HANDLE, "Memory was not initialized");

		device->GetHandle().bindImageMemory(image, memory, 0);
	}

	VulkanTextureResource::~VulkanTextureResource()
	{
		if (!isOwner) return;

		CP_EXPECT_MSG(device, "No VulkanDeviced associated to VulkanTextureResource");
		CP_EXPECT_MSG(image != VK_NULL_HANDLE, "Image shouldn't be null");
		CP_EXPECT_MSG(memory != VK_NULL_HANDLE, "Memory shouldn't be null");

		device->GetHandle().destroyImage(image);
		device->GetHandle().freeMemory(memory);
	}
}
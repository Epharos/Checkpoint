#include "pch.hpp"

#include "Helpers.hpp"
#include "../Resources/Material.hpp"
#include <QtCore/qjsonobject.h>

namespace Helper
{
	namespace Memory
	{
		uint32_t FindMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
		{
			vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			throw std::runtime_error("Failed to find suitable memory type!");
		}

		vk::Buffer CreateBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::DeviceMemory& bufferMemory)
		{
			vk::BufferCreateInfo bufferCreateInfo;
			bufferCreateInfo.size = size;
			bufferCreateInfo.usage = usage;
			bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

			vk::Buffer buffer = device.createBuffer(bufferCreateInfo);

			vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);

			vk::MemoryAllocateInfo memoryAllocateInfo;
			memoryAllocateInfo.allocationSize = memoryRequirements.size;
			memoryAllocateInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties);

			bufferMemory = device.allocateMemory(memoryAllocateInfo);
			device.bindBufferMemory(buffer, bufferMemory, 0);

			return buffer;
		}

		cp::Buffer CreateBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
		{
			cp::Buffer buffer;
			buffer.buffer = CreateBuffer(device, physicalDevice, size, usage, properties, buffer.memory);
			return buffer;
		}

		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, void* data)
		{
			MapMemory(device, memory, size, 0, data);
		}

		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, const void* data)
		{
			MapMemory(device, memory, size, 0, data);
		}

		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, vk::DeviceSize offset, void* data)
		{
			void* mappedMemory = device.mapMemory(memory, offset, size);
			memcpy(mappedMemory, data, size);
			device.unmapMemory(memory);
		}

		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, vk::DeviceSize offset, const void* data)
		{
			void* mappedMemory = device.mapMemory(memory, offset, size);
			memcpy(mappedMemory, data, size);
			device.unmapMemory(memory);
		}

		void CopyBuffer(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, vk::DeviceSize size)
		{
			vk::CommandBufferAllocateInfo allocInfo;
			allocInfo.level = vk::CommandBufferLevel::ePrimary;
			allocInfo.commandPool = commandPool;
			allocInfo.commandBufferCount = 1;

			vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

			vk::CommandBufferBeginInfo beginInfo;
			beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

			commandBuffer.begin(beginInfo);

			vk::BufferCopy copyRegion;
			copyRegion.size = size;
			commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

			commandBuffer.end();

			vk::SubmitInfo submitInfo;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			if (queue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
			{
				LOG_ERROR("Failed to submit copy buffer command");
				throw std::runtime_error("Failed to submit copy buffer command");
			}

			queue.waitIdle();

			device.freeCommandBuffers(commandPool, commandBuffer);
		}

		void DestroyBuffer(const vk::Device& device, const vk::Buffer& buffer, const vk::DeviceMemory& bufferMemory)
		{
			device.destroyBuffer(buffer);
			device.freeMemory(bufferMemory);
		}
	}

	namespace Format
	{
		vk::Format FindSupportedFormat(const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
		{
			for (vk::Format format : candidates)
			{
				vk::FormatProperties props = physicalDevice.getFormatProperties(format);

				if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
				{
					return format;
				}
				else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
				{
					return format;
				}
			}

			throw std::runtime_error("Failed to find supported format!");
		}

		vk::Format FindDepthFormat(const vk::PhysicalDevice& physicalDevice)
		{
			return FindSupportedFormat(physicalDevice, { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
		}
	}

	namespace File
	{
		std::vector<char> ReadShaderFile(const std::string& _path)
		{
			std::ifstream stream(_path, std::ios::ate | std::ios::binary);

			if (!stream.is_open())
			{
				throw std::runtime_error("Failed to open file: " + _path);
			}

			size_t fileSize = static_cast<size_t>(stream.tellg());

			std::vector<char> buffer(fileSize);
			stream.seekg(0);
			stream.read(buffer.data(), fileSize);

			stream.close();

			return buffer;
		}
	}
}

void Helper::Image::CreateImage(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory)
{
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(width, height, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = usage;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;

	if (device.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create image");
		throw std::runtime_error("Failed to create image");
	}

	vk::MemoryRequirements memRequirements;
	device.getImageMemoryRequirements(image, &memRequirements);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Helper::Memory::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

	if (device.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to allocate image memory");
		throw std::runtime_error("Failed to allocate image memory");
	}

	device.bindImageMemory(image, imageMemory, 0);
}

void Helper::Image::TransitionImageLayout(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::CommandBuffer commandBuffer = Helper::CommandBuffer::BeginSingleTimeCommands(device, commandPool);

	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlags();
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		LOG_ERROR("Unsupported layout transition");
		throw std::invalid_argument("Unsupported layout transition");
	}

	commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

	Helper::CommandBuffer::EndSingleTimeCommands(device, commandPool, queue, commandBuffer);
}

void Helper::Image::CopyBufferToImage(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Buffer& buffer, const vk::Image& image, uint32_t width, uint32_t height)
{
	vk::CommandBuffer commandBuffer = Helper::CommandBuffer::BeginSingleTimeCommands(device, commandPool);

	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D(0, 0, 0);
	region.imageExtent = vk::Extent3D(width, height, 1);

	commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

	Helper::CommandBuffer::EndSingleTimeCommands(device, commandPool, queue, commandBuffer);
}

void Helper::Image::CreateImageView(const vk::Device& device, const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView& imageView)
{
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = image;
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (device.createImageView(&viewInfo, nullptr, &imageView) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create image view");
		throw std::runtime_error("Failed to create image view");
	}
}

void Helper::Image::CreateSampler(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::Sampler& sampler)
{
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = vk::Filter::eNearest;
	samplerInfo.minFilter = vk::Filter::eNearest;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = physicalDevice.getProperties().limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (device.createSampler(&samplerInfo, nullptr, &sampler) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create sampler");
		throw std::runtime_error("Failed to create sampler");
	}
}

vk::CommandBuffer Helper::CommandBuffer::BeginSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool)
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer;
	if (device.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to allocate command buffer");
		throw std::runtime_error("Failed to allocate command buffer");
	}

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to begin command buffer");
		throw std::runtime_error("Failed to begin command buffer");
	}

	return commandBuffer;
}

void Helper::CommandBuffer::EndSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::CommandBuffer& commandBuffer)
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	if (queue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to submit command buffer");
		throw std::runtime_error("Failed to submit command buffer");
	}

	queue.waitIdle();

	device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

cp::ShaderStages Helper::Material::GetShaderStageFromString(const std::string& stage)
{
	if (stage == "Vertex")
		return cp::ShaderStages::Vertex;
	else if (stage == "Fragment")
		return cp::ShaderStages::Fragment;
	else if (stage == "Pixel")
		return cp::ShaderStages::Fragment; // Alias
	else if (stage == "Geometry")
		return cp::ShaderStages::Geometry;
	else if (stage == "Tessellation Control")
		return cp::ShaderStages::TessellationControl;
	else if (stage == "Tessellation Evaluation")
		return cp::ShaderStages::TessellationEvaluation;
	else if (stage == "Mesh")
		return cp::ShaderStages::Mesh;
	else if (stage == "Compute")
		return cp::ShaderStages::Compute;
	else
		throw std::invalid_argument("Invalid shader stage: " + stage);
}

std::string Helper::Material::GetShaderStageString(const cp::ShaderStages& stage)
{
	switch (stage)
	{
	case cp::ShaderStages::Vertex:
		return "Vertex";
	case cp::ShaderStages::Fragment:
		return "Fragment";
	case cp::ShaderStages::Geometry:
		return "Geometry";
	case cp::ShaderStages::TessellationControl:
		return "Tessellation Control";
	case cp::ShaderStages::TessellationEvaluation:
		return "Tessellation Evaluation";
	case cp::ShaderStages::Mesh:
		return "Mesh";
	case cp::ShaderStages::Compute:
		return "Compute";
	default:
		throw std::invalid_argument("Invalid shader stage");
	}
}

cp::MaterialFieldType Helper::Material::GetMaterialFieldTypeFromString(const std::string& type)
{
	if (type == "Boolean")
		return cp::MaterialFieldType::BOOL;
	else if (type == "Half")
		return cp::MaterialFieldType::HALF;
	else if (type == "Float")
		return cp::MaterialFieldType::FLOAT;
	else if (type == "Integer")
		return cp::MaterialFieldType::INT;
	else if (type == "Unsigned Integer")
		return cp::MaterialFieldType::UINT;
	else if (type == "Vector")
		return cp::MaterialFieldType::VECTOR;
	else if (type == "Matrix")
		return cp::MaterialFieldType::MATRIX;
	else
		throw std::invalid_argument("Invalid material field type: " + type);
}

std::string Helper::Material::GetMaterialFieldTypeString(const cp::MaterialFieldType& type)
{
	switch (type)
	{
	case cp::MaterialFieldType::BOOL:
		return "Boolean";
	case cp::MaterialFieldType::HALF:
		return "Half";
	case cp::MaterialFieldType::FLOAT:
		return "Float";
	case cp::MaterialFieldType::INT:
		return "Integer";
	case cp::MaterialFieldType::UINT:
		return "Unsigned Integer";
	case cp::MaterialFieldType::VECTOR:
		return "Vector";
	case cp::MaterialFieldType::MATRIX:
		return "Matrix";
	default:
		throw std::invalid_argument("Invalid material field type");
	}
}

cp::BindingType Helper::Material::GetMaterialBindingFromString(const std::string& binding)
{
	if (binding == "Uniform Buffer")
		return cp::BindingType::UNIFORM_BUFFER;
	else if (binding == "Storage Buffer")
		return cp::BindingType::STORAGE_BUFFER;
	else if (binding == "Texture")
		return cp::BindingType::TEXTURE;
	else
		throw std::invalid_argument("Invalid material binding: " + binding);
}

std::string Helper::Material::GetMaterialBindingString(const cp::BindingType& binding)
{
	switch (binding)
	{
	case cp::BindingType::UNIFORM_BUFFER:
		return "Uniform Buffer";
	case cp::BindingType::STORAGE_BUFFER:
		return "Storage Buffer";
	case cp::BindingType::TEXTURE:
		return "Texture";
	default:
		throw std::invalid_argument("Invalid material binding");
	}
}

vk::DescriptorType Helper::Material::GetDescriptorTypeFromBindingType(const cp::BindingType& binding)
{
	switch (binding)
	{
	case cp::BindingType::UNIFORM_BUFFER:
		return vk::DescriptorType::eUniformBuffer;
	case cp::BindingType::STORAGE_BUFFER:
		return vk::DescriptorType::eStorageBuffer;
	case cp::BindingType::TEXTURE:
		return vk::DescriptorType::eCombinedImageSampler;
	default:
		throw std::invalid_argument("Invalid material binding");
	}
}

vk::ShaderStageFlags Helper::Material::GetShaderStageFlags(const uint16_t& stages)
{
	vk::ShaderStageFlags stageFlags = static_cast<vk::ShaderStageFlagBits>(0);

	if (stages & static_cast<uint16_t>(cp::ShaderStages::Vertex))
		stageFlags |= vk::ShaderStageFlagBits::eVertex;

	if (stages & static_cast<uint16_t>(cp::ShaderStages::Fragment))
		stageFlags |= vk::ShaderStageFlagBits::eFragment;

	if (stages & static_cast<uint16_t>(cp::ShaderStages::Geometry))
		stageFlags |= vk::ShaderStageFlagBits::eGeometry;

	if (stages & static_cast<uint16_t>(cp::ShaderStages::TessellationControl))
		stageFlags |= vk::ShaderStageFlagBits::eTessellationControl;

	if (stages & static_cast<uint16_t>(cp::ShaderStages::TessellationEvaluation))
		stageFlags |= vk::ShaderStageFlagBits::eTessellationEvaluation;

	if (stages & static_cast<uint16_t>(cp::ShaderStages::Mesh))
		stageFlags |= vk::ShaderStageFlagBits::eMeshEXT;

	if (stages & static_cast<uint16_t>(cp::ShaderStages::Compute))
		stageFlags |= vk::ShaderStageFlagBits::eCompute;

	if (static_cast<uint32_t>(stageFlags) == 0)
	{
		LOG_ERROR("Invalid shader stage flags");
		throw std::invalid_argument("Invalid shader stage flags");
	}

	return stageFlags;
}



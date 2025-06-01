#include "pch.hpp"

#include "Helpers.hpp"
#include "../Resources/Material.hpp"
#include <QtCore/qjsonobject.h>

#include <slang/slang.h>
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>

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

		void DestroyBuffer(const vk::Device& device, cp::Buffer& buffer)
		{
			if (buffer.buffer)
			{
				DestroyBuffer(device, buffer.buffer, buffer.memory);
				buffer.buffer = VK_NULL_HANDLE;
				buffer.memory = VK_NULL_HANDLE;
			}
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

		std::string FileContentToString(const std::string& _path)
		{
			std::ifstream file(_path);

			if (!file.is_open())
			{
				LOG_ERROR("Failed to open file: " + _path);
				throw std::runtime_error("Failed to open file: " + _path);
			}

			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();
			return buffer.str();
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
		return cp::MaterialFieldType::Bool;
	else if (type == "Half")
		return cp::MaterialFieldType::Half;
	else if (type == "Float")
		return cp::MaterialFieldType::Float;
	else if (type == "Integer")
		return cp::MaterialFieldType::Int;
	else if (type == "Unsigned Integer")
		return cp::MaterialFieldType::UInt;
	else if (type == "Vector")
		return cp::MaterialFieldType::Vector;
	else if (type == "Matrix")
		return cp::MaterialFieldType::Matrix;
	else
		throw std::invalid_argument("Invalid material field type: " + type);
}

std::string Helper::Material::GetMaterialFieldTypeString(const cp::MaterialFieldType& type)
{
	switch (type)
	{
	case cp::MaterialFieldType::Bool:
		return "Boolean";
	case cp::MaterialFieldType::Half:
		return "Half";
	case cp::MaterialFieldType::Float:
		return "Float";
	case cp::MaterialFieldType::Int:
		return "Integer";
	case cp::MaterialFieldType::UInt:
		return "Unsigned Integer";
	case cp::MaterialFieldType::Vector:
		return "Vector";
	case cp::MaterialFieldType::Matrix:
		return "Matrix";
	default:
		throw std::invalid_argument("Invalid material field type");
	}
}

vk::DescriptorType Helper::Material::GetDescriptorTypeFromBindingType(const cp::ShaderResourceKind& bindingType)
{
	switch (bindingType)
	{
	case cp::ShaderResourceKind::ConstantBuffer:
		return vk::DescriptorType::eUniformBuffer;
	case cp::ShaderResourceKind::StructuredBuffer:
		return vk::DescriptorType::eStorageBuffer;
	case cp::ShaderResourceKind::CombinedImageSampler:
		return vk::DescriptorType::eCombinedImageSampler;
	case cp::ShaderResourceKind::TextureResource:
		return vk::DescriptorType::eSampledImage;
	case cp::ShaderResourceKind::Sampler:
		return vk::DescriptorType::eSampler;
	default:
		LOG_ERROR("Invalid shader resource kind");
		throw std::invalid_argument("Invalid shader resource kind");
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

QWidget* Helper::Material::CreateMaterialFieldWidget(QWidget* _parent, const cp::ShaderField& field, void* dataPtr)
{
	if (field.typeName.compare("float") == 0) return new cp::Float(reinterpret_cast<float*>(dataPtr), field.name.c_str());
	if (field.typeName.compare("double") == 0) return new cp::Double(reinterpret_cast<double*>(dataPtr), field.name.c_str());
	if (field.typeName.compare("int") == 0) return new cp::Int32(reinterpret_cast<int*>(dataPtr), field.name.c_str());
	if (field.typeName.compare("uint") == 0) return new cp::UInt32(reinterpret_cast<uint32_t*>(dataPtr), field.name.c_str());
	if (field.typeName.compare("vector") == 0)
	{
		if (field.vectorType.compare("float") == 0)
		{
			switch (field.vectorSize)
			{
			case 2: return new cp::Float2(reinterpret_cast<glm::vec2*>(dataPtr), field.name.c_str());
			case 3: return new cp::Float3(reinterpret_cast<glm::vec3*>(dataPtr), field.name.c_str());
			case 4: return new cp::Float4(reinterpret_cast<glm::vec4*>(dataPtr), field.name.c_str());
			}
		}
	}

	//LOG_WARNING("Unsupported material field type: " + field.typeName);
	return nullptr;
}

void Helper::Slang::DeepnessToString(const std::string& text, uint8_t deepness, std::stringstream& ss)
{
	for (uint8_t i = 0; i < deepness; i++)
		ss << "   ";

	ss << text << "\n";
}

std::string Helper::Slang::VariableToString(slang::VariableLayoutReflection* variable, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Name: " + std::string(variable->getName()), deepness, ss);
	ss << TypeToString(variable->getTypeLayout(), deepness + 1);

	return ss.str();
}

std::string Helper::Slang::TypeToString(slang::TypeLayoutReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Type: " + std::string(type->getName()), deepness, ss);

	auto PrintSize = [&](slang::TypeLayoutReflection* type, SlangParameterCategory category)
		{
			size_t size = type->getSize();

			if (size == ~size_t(0))
			{
				return std::string("Size: Dynamic");
			}
			else
			{
				return "Size: " + std::to_string(size);
			}

			return std::string("");
		};

	for (int i = 0; i < type->getCategoryCount(); i++)
	{
		auto layoutUnit = (SlangParameterCategory)type->getCategoryByIndex(i);

		DeepnessToString(PrintSize(type, layoutUnit), deepness + 1, ss);

		if (type->getSize() > 0)
		{
			auto alignment = type->getAlignment(layoutUnit);
			auto stride = type->getStride(layoutUnit);
			auto alignmentString = "Alignment: " + std::to_string(alignment) + " bytes";
			auto strideString = "Stride: " + std::to_string(stride) + " bytes";
			DeepnessToString(alignmentString, deepness + 1, ss);
			DeepnessToString(strideString, deepness + 1, ss);
		}
	}

	switch (type->getKind())
	{
	case slang::TypeReflection::Kind::Struct:
		DeepnessToString(StructureTypeToString(type, deepness + 1), 0, ss);
		break;
	case slang::TypeReflection::Kind::Array:
		DeepnessToString(ArrayTypeToString(type, deepness + 1), 0, ss);
		break;
	case slang::TypeReflection::Kind::Matrix:
		DeepnessToString(MatrixTypeToString(type, deepness + 1), 0, ss);
		break;
	case slang::TypeReflection::Kind::Vector:
		DeepnessToString(VectorTypeToString(type, deepness + 1), 0, ss);
		break;
	case slang::TypeReflection::Kind::Scalar:
		DeepnessToString(ScalarTypeToString(type, deepness + 1), 0, ss);
		break;
	case slang::TypeReflection::Kind::Resource:
		DeepnessToString(ResourceTypeToString(type, deepness + 1), 0, ss);
		break;
	case slang::TypeReflection::Kind::ConstantBuffer:
	case slang::TypeReflection::Kind::ShaderStorageBuffer:
	case slang::TypeReflection::Kind::ParameterBlock:
	case slang::TypeReflection::Kind::TextureBuffer:
		DeepnessToString(SingleElementContainerTypeToString(type, deepness + 1), 0, ss);
		break;
	default:
		DeepnessToString("Unknown type kind: " + std::to_string(static_cast<int>(type->getKind())) + "\n", deepness + 1, ss);
		break;
	}

	return ss.str();
}

std::string Helper::Slang::VariableToString(slang::VariableReflection* variable, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Name: " + std::string(variable->getName()), deepness, ss);
	ss << TypeToString(variable->getType(), deepness);

	return ss.str();
}

std::string Helper::Slang::TypeToString(slang::TypeReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Type: " + std::string(type->getName()), deepness + 1, ss);

	switch (type->getKind())
	{
	case slang::TypeReflection::Kind::Struct:
		DeepnessToString(StructureTypeToString(type, 0), deepness + 1, ss);
		break;
	case slang::TypeReflection::Kind::Array:
		DeepnessToString(ArrayTypeToString(type, 0), deepness + 1, ss);
		break;
	case slang::TypeReflection::Kind::Matrix:
		DeepnessToString(MatrixTypeToString(type, 0), deepness + 1, ss);
		break;
	case slang::TypeReflection::Kind::Vector:
		DeepnessToString(VectorTypeToString(type, 0), deepness + 1, ss);
		break;
	case slang::TypeReflection::Kind::Scalar:
		DeepnessToString(ScalarTypeToString(type, 0), deepness + 1, ss);
		break;
	case slang::TypeReflection::Kind::Resource:
		DeepnessToString(ResourceTypeToString(type, 0), deepness + 1, ss);
		break;
	case slang::TypeReflection::Kind::ConstantBuffer:
	case slang::TypeReflection::Kind::ShaderStorageBuffer:
	case slang::TypeReflection::Kind::ParameterBlock:
	case slang::TypeReflection::Kind::TextureBuffer:
		DeepnessToString(SingleElementContainerTypeToString(type, 0), deepness + 1, ss);
		break;
	default:
		DeepnessToString("Unknown type kind: " + std::to_string(static_cast<int>(type->getKind())) + "\n", deepness + 1, ss);
		break;
	}

	return ss.str();
}

std::string Helper::Slang::ScalarTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	switch (type->getScalarType())
	{
	case slang::TypeReflection::ScalarType::Float16:
		DeepnessToString("Scalar Type: Float16", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Float32:
		DeepnessToString("Scalar Type: Float32", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Float64:
		DeepnessToString("Scalar Type: Float64", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Int8:
		DeepnessToString("Scalar Type: Int8", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Int16:
		DeepnessToString("Scalar Type: Int16", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Int32:
		DeepnessToString("Scalar Type: Int32", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Int64:
		DeepnessToString("Scalar Type: Int64", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::UInt8:
		DeepnessToString("Scalar Type: UInt8", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::UInt16:
		DeepnessToString("Scalar Type: UInt16", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::UInt32:
		DeepnessToString("Scalar Type: UInt32", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::UInt64:
		DeepnessToString("Scalar Type: UInt64", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Bool:
		DeepnessToString("Scalar Type: Bool", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Void:
		DeepnessToString("Scalar Type: String", deepness, ss);
		break;
	default:
		DeepnessToString("Unknown scalar type: " + std::to_string(static_cast<int>(type->getScalarType())) + "\n", deepness, ss);
		break;
	}

	return ss.str();
}

std::string Helper::Slang::StructureTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Fields:", deepness, ss);

	for (uint32_t i = 0; i < type->getFieldCount(); i++)
	{
		auto field = type->getFieldByIndex(i);
		ss << VariableToString(field, deepness + 1);
	}

	return ss.str();
}

std::string Helper::Slang::ArrayTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	size_t size = type->getElementCount();
	if (size == ~size_t(0))
		DeepnessToString("Array Size: Dynamic", deepness, ss);
	else
		DeepnessToString("Array Size: " + std::to_string(size), deepness, ss);

	DeepnessToString("Element Type: " + std::string(type->getElementTypeLayout()->getName()), deepness, ss);
	ss << TypeToString(type->getElementTypeLayout(), deepness + 1) << "\n";

	return ss.str();
}

std::string Helper::Slang::VectorTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Vector Length: " + std::to_string(type->getElementCount()), deepness, ss);
	DeepnessToString("Element Type: " + std::string(type->getElementTypeLayout()->getName()), deepness, ss);
	ss << TypeToString(type->getElementTypeLayout(), deepness + 1) << "\n";

	return ss.str();
}

std::string Helper::Slang::MatrixTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Matrix Type: " + std::to_string(type->getColumnCount()) + "x" + std::to_string(type->getRowCount()), deepness, ss);
	DeepnessToString("Element Type: " + std::string(type->getElementTypeLayout()->getName()), deepness, ss);

	ss << TypeToString(type->getElementTypeLayout(), deepness + 1) << "\n";
	return ss.str();
}

std::string Helper::Slang::ResourceTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	auto ShapeToString = [&](SlangResourceShape shape, uint8_t deepness)
		{
			std::stringstream ss;

			switch (shape)
			{
			case SlangResourceShape::SLANG_TEXTURE_1D:
				ss << "Texture 1D";
				break;
			case SlangResourceShape::SLANG_TEXTURE_2D:
				ss << "Texture 2D";
				break;
			case SlangResourceShape::SLANG_TEXTURE_3D:
				ss << "Texture 3D";
				break;
			case SlangResourceShape::SLANG_TEXTURE_CUBE:
				ss << "Texture Cube";
				break;
			case SlangResourceShape::SLANG_TEXTURE_BUFFER:
				ss << "Texture Buffer";
				break;
			case SlangResourceShape::SLANG_TEXTURE_1D_ARRAY:
				ss << "Texture 1D Array";
				break;
			case SlangResourceShape::SLANG_TEXTURE_2D_ARRAY:
				ss << "Texture 2D Array";
				break;
			case SlangResourceShape::SLANG_TEXTURE_CUBE_ARRAY:
				ss << "Texture Cube Array";
				break;
			case SlangResourceShape::SLANG_STRUCTURED_BUFFER:
				ss << "Structured Buffer";
				break;
			}

			return ss.str();
		};
	auto AccessToString = [&](SlangResourceAccess access, uint8_t deepness)
		{
			std::stringstream ss;
			switch (access)
			{
			case SlangResourceAccess::SLANG_RESOURCE_ACCESS_READ:
				ss << "Read";
				break;
			case SlangResourceAccess::SLANG_RESOURCE_ACCESS_WRITE:
				ss << "Write";
				break;
			case SlangResourceAccess::SLANG_RESOURCE_ACCESS_READ_WRITE:
				ss << "Read Write";
				break;
			}
			return ss.str();
		};

	DeepnessToString("Resource Shape: " + ShapeToString(type->getResourceShape(), deepness), deepness, ss);
	DeepnessToString("Resource Access: " + AccessToString(type->getResourceAccess(), deepness), deepness, ss);
	DeepnessToString("Resource Type: " + std::string(type->getResourceResultType()->getName()), deepness, ss);
	ss << TypeToString(type->getResourceResultType(), deepness + 1) << "\n";

	return ss.str();
}

std::string Helper::Slang::SingleElementContainerTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Container Type: " + std::string(type->getElementTypeLayout()->getName()), deepness, ss);
	ss << TypeToString(type->getElementTypeLayout(), deepness + 1) << "\n";

	return ss.str();
}

std::string Helper::Slang::ScalarTypeToString(slang::TypeReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	switch (type->getScalarType())
	{
	case slang::TypeReflection::ScalarType::Float16:
		DeepnessToString("Scalar Type: Float16", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Float32:
		DeepnessToString("Scalar Type: Float32", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Float64:
		DeepnessToString("Scalar Type: Float64", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Int8:
		DeepnessToString("Scalar Type: Int8", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Int16:
		DeepnessToString("Scalar Type: Int16", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Int32:
		DeepnessToString("Scalar Type: Int32", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Int64:
		DeepnessToString("Scalar Type: Int64", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::UInt8:
		DeepnessToString("Scalar Type: UInt8", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::UInt16:
		DeepnessToString("Scalar Type: UInt16", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::UInt32:
		DeepnessToString("Scalar Type: UInt32", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::UInt64:
		DeepnessToString("Scalar Type: UInt64", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Bool:
		DeepnessToString("Scalar Type: Bool", deepness, ss);
		break;
	case slang::TypeReflection::ScalarType::Void:
		DeepnessToString("Scalar Type: String", deepness, ss);
		break;
	default:
		DeepnessToString("Unknown scalar type: " + std::to_string(static_cast<int>(type->getScalarType())) + "\n", deepness, ss);
		break;
	}

	return ss.str();
}

std::string Helper::Slang::StructureTypeToString(slang::TypeReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Fields:", deepness, ss);

	for (uint32_t i = 0; i < type->getFieldCount(); i++)
	{
		slang::VariableReflection* field = type->getFieldByIndex(i);
		ss << VariableToString(field, deepness + 1);
	}

	return ss.str();
}

std::string Helper::Slang::ArrayTypeToString(slang::TypeReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	size_t size = type->getElementCount();
	if (size == ~size_t(0))
		DeepnessToString("Array Size: Dynamic", deepness, ss);
	else
		DeepnessToString("Array Size: " + std::to_string(size), deepness, ss);

	DeepnessToString("Element Type: " + std::string(type->getElementType()->getName()), deepness, ss);
	ss << TypeToString(type->getElementType(), deepness + 1) << "\n";

	return ss.str();
}

std::string Helper::Slang::VectorTypeToString(slang::TypeReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Vector Length: " + std::to_string(type->getElementCount()), deepness, ss);
	DeepnessToString("Element Type: " + std::string(type->getElementType()->getName()), deepness, ss);
	ss << TypeToString(type->getElementType(), deepness + 1) << "\n";

	return ss.str();
}

std::string Helper::Slang::MatrixTypeToString(slang::TypeReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Matrix Type: " + std::to_string(type->getColumnCount()) + "x" + std::to_string(type->getRowCount()), deepness, ss);
	DeepnessToString("Element Type: " + std::string(type->getElementType()->getName()), deepness, ss);

	ss << TypeToString(type->getElementType(), deepness + 1) << "\n";
	return ss.str();
}

std::string Helper::Slang::ResourceTypeToString(slang::TypeReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	auto ShapeToString = [&](SlangResourceShape shape, uint8_t deepness)
		{
			std::stringstream ss;

			switch (shape)
			{
			case SlangResourceShape::SLANG_TEXTURE_1D:
				ss << "Texture 1D";
				break;
			case SlangResourceShape::SLANG_TEXTURE_2D:
				ss << "Texture 2D";
				break;
			case SlangResourceShape::SLANG_TEXTURE_3D:
				ss << "Texture 3D";
				break;
			case SlangResourceShape::SLANG_TEXTURE_CUBE:
				ss << "Texture Cube";
				break;
			case SlangResourceShape::SLANG_TEXTURE_BUFFER:
				ss << "Texture Buffer";
				break;
			case SlangResourceShape::SLANG_TEXTURE_1D_ARRAY:
				ss << "Texture 1D Array";
				break;
			case SlangResourceShape::SLANG_TEXTURE_2D_ARRAY:
				ss << "Texture 2D Array";
				break;
			case SlangResourceShape::SLANG_TEXTURE_CUBE_ARRAY:
				ss << "Texture Cube Array";
				break;
			case SlangResourceShape::SLANG_STRUCTURED_BUFFER:
				ss << "Structured Buffer";
				break;
			}

			return ss.str();
		};
	auto AccessToString = [&](SlangResourceAccess access, uint8_t deepness)
		{
			std::stringstream ss;
			switch (access)
			{
			case SlangResourceAccess::SLANG_RESOURCE_ACCESS_READ:
				ss << "Read";
				break;
			case SlangResourceAccess::SLANG_RESOURCE_ACCESS_WRITE:
				ss << "Write";
				break;
			case SlangResourceAccess::SLANG_RESOURCE_ACCESS_READ_WRITE:
				ss << "Read Write";
				break;
			}
			return ss.str();
		};

	DeepnessToString("Resource Shape: " + ShapeToString(type->getResourceShape(), deepness), deepness, ss);
	DeepnessToString("Resource Access: " + AccessToString(type->getResourceAccess(), deepness), deepness, ss);
	DeepnessToString("Resource Type: " + std::string(type->getResourceResultType()->getName()), deepness, ss);
	ss << TypeToString(type->getResourceResultType(), deepness + 1) << "\n";

	return ss.str();
}

std::string Helper::Slang::SingleElementContainerTypeToString(slang::TypeReflection* type, uint8_t deepness)
{
	std::stringstream ss;

	DeepnessToString("Container Type: " + std::string(type->getElementType()->getName()), deepness, ss);
	ss << TypeToString(type->getElementType(), deepness + 1) << "\n";

	return ss.str();
}

#include "pch.hpp"

#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

cp::Texture::~Texture()
{
	auto device = context->GetDevice();

	device.destroySampler(sampler);
	device.destroyImageView(imageView);
	device.freeMemory(imageMemory);
	device.destroyImage(image);
}

std::shared_ptr<cp::Texture> cp::Texture::LoadTexture(const cp::VulkanContext& _context, const std::string& _path)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(&_context);
	stbi_uc* pixels = stbi_load(_path.c_str(), &texture->width, &texture->height, &texture->channels, STBI_rgb_alpha);

	if (!pixels)
	{
		LOG_ERROR("Failed to load texture image: " + _path);
		return nullptr;
	}

	vk::DeviceSize imageSize = texture->width * texture->height * 4;

	vk::Buffer buffer;
	vk::DeviceMemory bufferMemory;
	buffer = Helper::Memory::CreateBuffer(_context.GetDevice(), _context.GetPhysicalDevice(), imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferMemory);
	Helper::Memory::MapMemory(_context.GetDevice(), bufferMemory, imageSize, (void*)pixels);

	stbi_image_free(pixels);

	Helper::Image::CreateImage(_context.GetDevice(), _context.GetPhysicalDevice(), texture->width, texture->height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, texture->image, texture->imageMemory);
	Helper::Image::TransitionImageLayout(_context.GetDevice(), _context.GetCommandPool(), _context.GetDevice().getQueue(_context.GetQueueFamilyIndices().graphicsFamily.value(), 0), texture->image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	Helper::Image::CopyBufferToImage(_context.GetDevice(), _context.GetCommandPool(), _context.GetDevice().getQueue(_context.GetQueueFamilyIndices().graphicsFamily.value(), 0), buffer, texture->image, texture->width, texture->height);
	Helper::Image::TransitionImageLayout(_context.GetDevice(), _context.GetCommandPool(), _context.GetDevice().getQueue(_context.GetQueueFamilyIndices().graphicsFamily.value(), 0), texture->image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	Helper::Memory::DestroyBuffer(_context.GetDevice(), buffer, bufferMemory);

	Helper::Image::CreateImageView(_context.GetDevice(), texture->image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, texture->imageView);
	Helper::Image::CreateSampler(_context.GetDevice(), _context.GetPhysicalDevice(), texture->sampler);

	LOG_INFO(MF("Texture: ", _path, " loaded (", texture->width, "x", texture->height, ")"));

	return texture;
}

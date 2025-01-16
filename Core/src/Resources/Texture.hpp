#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

namespace Resource
{
	class Texture
	{
	private:
		int width;
		int height;
		int channels;

		vk::Image image;
		vk::DeviceMemory imageMemory;

		vk::ImageView imageView;
		vk::Sampler sampler;

		const Context::VulkanContext* context;

	public:
		Texture(const Context::VulkanContext* _context) : context(_context) {}
		~Texture();

		static Texture* LoadTexture(const Context::VulkanContext& _context, const std::string& _path);

		inline constexpr const vk::Image GetImage() const { return image; }
		inline constexpr const vk::ImageView GetImageView() const { return imageView; }
		inline constexpr const vk::Sampler GetSampler() const { return sampler; }

		inline constexpr int GetWidth() const { return width; }
		inline constexpr int GetHeight() const { return height; }
		inline constexpr int GetChannels() const { return channels; }
	};
}
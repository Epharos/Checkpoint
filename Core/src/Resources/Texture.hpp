#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

namespace cp
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

		const cp::VulkanContext* context;

	public:
		Texture(const cp::VulkanContext* _context) : context(_context) {}
		~Texture();

		static std::shared_ptr<Texture> LoadTexture(const cp::VulkanContext& _context, const std::string& _path);

		inline constexpr const vk::Image GetImage() const { return image; }
		inline constexpr const vk::ImageView GetImageView() const { return imageView; }
		inline constexpr const vk::Sampler GetSampler() const { return sampler; }

		inline constexpr int GetWidth() const { return width; }
		inline constexpr int GetHeight() const { return height; }
		inline constexpr int GetChannels() const { return channels; }
	};
}
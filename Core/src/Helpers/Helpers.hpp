#pragma once

#include "../pch.hpp"
#include "../Util/Buffer.hpp"

namespace cp
{
	enum class ShaderStages : uint16_t;
	enum class MaterialFieldType : uint8_t;
	enum class BindingType : uint8_t;
}

namespace Helper
{
	namespace Memory
	{
		uint32_t FindMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
		[[nodiscard]] vk::Buffer CreateBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::DeviceMemory& bufferMemory);
		[[nodiscard]] cp::Buffer CreateBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, void* data);
		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, const void* data);

		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, vk::DeviceSize offset, void* data);
		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, vk::DeviceSize offset, const void* data);
		void CopyBuffer(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, vk::DeviceSize size);
		void DestroyBuffer(const vk::Device& device, const vk::Buffer& buffer, const vk::DeviceMemory& bufferMemory);
	}

	namespace Format
	{
		vk::Format FindSupportedFormat(const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format FindDepthFormat(const vk::PhysicalDevice& physicalDevice);
	}

	namespace File
	{
		std::vector<char> ReadShaderFile(const std::string& _path);
	}

	namespace Image
	{
		void CreateImage(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
		void TransitionImageLayout(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void CopyBufferToImage(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Buffer& buffer, const vk::Image& image, uint32_t width, uint32_t height);
		void CreateImageView(const vk::Device& device, const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView& imageView);
		void CreateSampler(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::Sampler& sampler);
	}

	namespace CommandBuffer
	{
		vk::CommandBuffer BeginSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool);
		void EndSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::CommandBuffer& commandBuffer);
	}

	namespace Hash
	{
		template<typename T>
		void CombineHashes(size_t& seed, const T& value)
		{
			std::hash<T> hasher;
			seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		template <typename... Args>
		struct TupleHash 
		{
			std::size_t operator()(const std::tuple<Args...>& tuple) const 
			{
				return HashTuple(tuple, std::index_sequence_for<Args...>{});
			}

		private:
			template <typename TupleType, std::size_t... Index>
			std::size_t HashTuple(const TupleType& tuple, std::index_sequence<Index...>) const 
			{
				std::size_t seed = 0;
				(..., CombineHashes(seed, std::get<Index>(tuple)));
				return seed;
			}
		};
	}

	namespace Material
	{
		cp::ShaderStages GetShaderStageFromString(const std::string& stage);
		std::string GetShaderStageString(const cp::ShaderStages& stage);

		cp::MaterialFieldType GetMaterialFieldTypeFromString(const std::string& type);
		std::string GetMaterialFieldTypeString(const cp::MaterialFieldType& type);

		cp::BindingType GetMaterialBindingFromString(const std::string& binding);
		std::string GetMaterialBindingString(const cp::BindingType& binding);

		vk::DescriptorType GetDescriptorTypeFromBindingType(const cp::BindingType& binding);
		vk::ShaderStageFlags GetShaderStageFlags(const uint16_t& stages);
	}
}
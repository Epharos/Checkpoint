#pragma once

#include "../pch.hpp"
#include "../Util/Buffer.hpp"

#ifdef IN_EDITOR
#include <QtWidgets/qwidget.h>
#endif

namespace cp
{
	enum class ShaderStages : uint16_t;
	enum class MaterialFieldType : uint8_t;
	enum class ShaderResourceKind : uint8_t;
	struct ShaderField;
}

namespace slang
{
	struct VariableReflection;
	struct VariableLayoutReflection;
	struct TypeReflection;
	struct TypeLayoutReflection;
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
		std::string FileContentToString(const std::string& _path);
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

		vk::DescriptorType GetDescriptorTypeFromBindingType(const cp::ShaderResourceKind& bindingType);
		vk::ShaderStageFlags GetShaderStageFlags(const uint16_t& stages);

#ifdef IN_EDITOR
		QWidget* CreateMaterialFieldWidget(QWidget* _parent, const cp::ShaderField& field, void* dataPtr);
#endif
	}

	namespace Slang
	{
		void DeepnessToString(const std::string& text, uint8_t deepness, std::stringstream& ss);

		std::string VariableToString(slang::VariableLayoutReflection* variable, uint8_t deepness = 0);
		std::string TypeToString(slang::TypeLayoutReflection* type, uint8_t deepness = 0);
		std::string ScalarTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness = 0);
		std::string StructureTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness = 0);
		std::string ArrayTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness = 0);
		std::string VectorTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness = 0);
		std::string MatrixTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness = 0);
		std::string ResourceTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness = 0);
		std::string SingleElementContainerTypeToString(slang::TypeLayoutReflection* type, uint8_t deepness = 0);
		
		std::string VariableToString(slang::VariableReflection* variable, uint8_t deepness = 0);
		std::string TypeToString(slang::TypeReflection* type, uint8_t deepness = 0);
		std::string ScalarTypeToString(slang::TypeReflection* type, uint8_t deepness = 0);
		std::string StructureTypeToString(slang::TypeReflection* type, uint8_t deepness = 0);
		std::string ArrayTypeToString(slang::TypeReflection* type, uint8_t deepness = 0);
		std::string VectorTypeToString(slang::TypeReflection* type, uint8_t deepness = 0);
		std::string MatrixTypeToString(slang::TypeReflection* type, uint8_t deepness = 0);
		std::string ResourceTypeToString(slang::TypeReflection* type, uint8_t deepness = 0);
		std::string SingleElementContainerTypeToString(slang::TypeReflection* type, uint8_t deepness = 0);
	}
}
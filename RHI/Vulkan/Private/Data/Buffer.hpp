#pragma once

#include "../pch.hpp"

#include <RHI/Data.hpp>

namespace cp
{
	class Device;
	class ILogger;

	class Buffer final : public IBuffer
	{
	public:
		Buffer(ILogger& _logger, const BufferInfo& _info, Device& _device);
		~Buffer() override;

		void* Map() override;
		void Unmap() override;

		[[nodiscard]] vk::Buffer& GetHandle() { return buffer; }
		[[nodiscard]] const vk::Buffer& GetHandle() const { return buffer; }

		[[nodiscard]] vk::DeviceMemory& GetMemory() { return memory; }
		[[nodiscard]] const vk::DeviceMemory& GetMemory() const { return memory; }

	private:
		void Initialize();
		void Cleanup() const;

	private:
		vk::Buffer buffer{ VK_NULL_HANDLE };
		vk::DeviceMemory memory{ VK_NULL_HANDLE };
		void* mappedMemory = nullptr;

		Device& device;
		ILogger& logger;
	};
}

#pragma once

#include <cstddef>
#include <cstdint>

namespace cp
{
	enum class BufferUsage : uint32_t
	{
		None = 0,

		TransferSrc = 1u << 0,
		TransferDst = 1u << 1,

		Vertex = 1u << 2,
		Index = 1u << 3,
		Uniform = 1u << 4,
		Storage = 1u << 5,
		Indirect = 1u << 6,
	};

	constexpr BufferUsage operator|(BufferUsage _a, BufferUsage _b)
	{
		return static_cast<BufferUsage>(static_cast<uint32_t>(_a) | static_cast<uint32_t>(_b));
	}

	constexpr BufferUsage& operator|=(BufferUsage& _a, const BufferUsage _b)
	{
		_a = _a | _b;
		return _a;
	}

	struct BufferInfo
	{
		size_t sizeBytes = 0;
		BufferUsage usage = BufferUsage::None;

		/**
		 * @brief Hint for memory placement.
		 *        CPU-visible buffers are useful for upload/readback, but can be slower on some backends.
		 */
		bool cpuVisible = false;
	};

	class IBuffer
	{
	public:
		explicit IBuffer(const BufferInfo& _info) : info(_info) {};
		virtual ~IBuffer() = default;

		[[nodiscard]] size_t GetSizeBytes() const { return info.sizeBytes; };
		[[nodiscard]] BufferUsage GetUsage() const { return info.usage; };

		virtual void* Map() = 0;
		virtual void Unmap() = 0;

		[[nodiscard]] const BufferInfo& GetBufferInfo() const { return info; };

	protected:
		const BufferInfo info;
	};
}

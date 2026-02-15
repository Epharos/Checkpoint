#pragma once

#include <cstdint>

namespace cp
{
	enum class RHIQueueType : uint8_t
	{
		Graphics,
		Compute,
		Transfer,
	};

	struct RHISubmitInfo
	{
		// TODO : Add command buffers, semaphores, etc.
	};

	class RHIQueue
	{
	public:
		virtual ~RHIQueue() = default;

		virtual void Submit(const RHISubmitInfo& _submitInfo) = 0;
		virtual void WaitIdle() = 0;

		virtual RHIQueueType GetType() const = 0;
	};
}
#pragma once

#include <cstdint>

namespace cp
{
	enum class IQueueType : uint8_t
	{
		Graphics,
		Compute,
		Transfer,
	};

	struct ISubmitInfo
	{
		// TODO : Add command buffers, semaphores, etc.
	};

	class IQueue
	{
	public:
		virtual ~IQueue() = default;

		virtual void Submit(const ISubmitInfo& _submitInfo) = 0;
		virtual void WaitIdle() = 0;

		virtual IQueueType GetType() const = 0;
	};
}
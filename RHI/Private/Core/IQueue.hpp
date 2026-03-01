#pragma once

#include <cstdint>

namespace cp
{
	class ITimelineSemaphore;

	enum class QueueType : uint8_t
	{
		Graphics,
		Compute,
		Transfer,
	};

	struct SubmitInfo
	{
		// std::vector<ICommandBuffer*> commandBuffers;

		struct WaitInfo
		{
			ITimelineSemaphore* semaphore;
			size_t value;
			// TODO : Add Pipeline Stage (Vulkan specific but can simply be ignored for DX12 and others)
		};

		struct SignalInfo
		{
			ITimelineSemaphore* semaphore;
			size_t value;
		};

		std::vector<WaitInfo> waitInfos;
		std::vector<SignalInfo> signalInfos;
	};

	class IQueue
	{
	public:
		virtual ~IQueue() = default;

		virtual void Submit(const SubmitInfo& _submitInfo) = 0;
		virtual void WaitIdle() = 0;

		[[nodiscard]] virtual QueueType GetType() const = 0;
	};
}
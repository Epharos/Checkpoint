#pragma once

#include "../pch.hpp"

#include <RHI/Core.hpp>

#include <mutex>

namespace cp
{
	struct VulkanQueueFamilies
	{
		/** Index of the graphics queue family. Supports draw, barrier and present operations. */
		uint32_t graphics = std::numeric_limits<uint32_t>::max();
		/** Index of the compute queue family. Supports dispatch operations. */
		uint32_t compute = std::numeric_limits<uint32_t>::max();
		/** Index of the transfer queue family. Supports copy operations. */
		uint32_t transfer = std::numeric_limits<uint32_t>::max();
		/** Index of the present queue family. May be the same as graphics. */
		uint32_t present = std::numeric_limits<uint32_t>::max();
	};

	class Queue final : public IQueue
	{
	public:
		Queue(vk::Queue _queue, uint32_t _familyIndex, QueueType _type);
		~Queue() override;

		void Submit(const SubmitInfo& _submitInfo) override;
		void WaitIdle() override;

		[[nodiscard]] QueueType GetType() const override { return type; }

		[[nodiscard]] uint32_t GetFamilyIndex() const { return familyIndex; }

		// Locked bridge submits — used by the Swapchain to synchronize binary/timeline semaphores
		void SubmitBinaryToTimeline(
			vk::Semaphore _binaryWait,
			vk::Semaphore _timelineSignal,
			uint64_t _timelineSignalValue
		);
		void SubmitTimelineToBinary(
			vk::Semaphore _timelineWait,
			uint64_t _timelineWaitValue,
			vk::Semaphore _binarySignal
		);
		vk::Result Present(const vk::PresentInfoKHR& _presentInfo);

	private:
		vk::Queue queue;
		uint32_t familyIndex;
		QueueType type;
		std::mutex submitMutex;
	};
}
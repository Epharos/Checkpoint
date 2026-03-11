#pragma once

#include <vector>

#include "../Data/Enums.hpp"

namespace cp
{
	enum class PipelineStage : uint32_t;
	class ICommandBuffer;
	class ITimelineSemaphore;

	enum class QueueType : uint8_t
	{
		Graphics,
		Compute,
		Transfer,
	};

	struct SubmitInfo
	{
		std::vector<ICommandBuffer*> commandBuffers;

		struct WaitInfo
		{
			/** The timeline semaphore to wait on before executing the command buffers. */
			ITimelineSemaphore* semaphore;
			/** The semaphore value to wait for. */
			size_t value;
			/** The pipeline stage at which the wait occurs. */
			PipelineStage stage = PipelineStage::AllGraphics;
		};

		struct SignalInfo
		{
			/** The timeline semaphore to signal after execution completes. */
			ITimelineSemaphore* semaphore;
			/** The value to signal on the semaphore. Must be strictly greater than its current pending value. */
			size_t value;
		};

		/** Semaphores and values the queue must wait on before executing the command buffers. */
		std::vector<WaitInfo> waitInfos;
		/** Semaphores and values to signal after all command buffers have finished executing. */
		std::vector<SignalInfo> signalInfos;
	};

	class IQueue
	{
	public:
		virtual ~IQueue() = default;

		/**
		* @brief Submits command buffers to the queue for execution, with optional wait and signal synchronization.
		*
		* @param _submitInfo The submission descriptor containing command buffers, wait and signal semaphore infos.
		*/
		virtual void Submit(const SubmitInfo& _submitInfo) = 0;

		/**
		* @brief Blocks the CPU until all work submitted to this queue has completed.
		*/
		virtual void WaitIdle() = 0;

		/**
		* @brief Returns the type of this queue (Graphics, Compute, Transfer).
		*
		* @return The queue type.
		*/
		[[nodiscard]] virtual QueueType GetType() const = 0;
	};
}

#include "../pch.hpp"

#include "Queue.hpp"

#include "CommandBuffer.hpp"
#include "../Synchro/TimelineSemaphore.hpp"

namespace cp
{
	Queue::Queue(const vk::Queue _queue, const uint32_t _familyIndex, const QueueType _type)
		: queue(_queue), familyIndex(_familyIndex), type(_type)
	{
	}

	Queue::~Queue() = default;

	void Queue::Submit(const SubmitInfo& _submitInfo)
	{
		std::vector<vk::CommandBuffer> commandBuffers;

		for (const ICommandBuffer* commandBuffer : _submitInfo.commandBuffers)
		{
			auto* upcastCommandBuffer = dynamic_cast<const CommandBuffer*>(commandBuffer);
			commandBuffers.push_back(upcastCommandBuffer->GetHandle());
		}

		std::vector<vk::Semaphore> waitSemaphores;
		std::vector<size_t> waitValues;
		std::vector<vk::PipelineStageFlags> waitStages;

		for (const SubmitInfo::WaitInfo& waitInfo : _submitInfo.waitInfos)
		{
			auto* timelineSemaphore = dynamic_cast<TimelineSemaphore*>(waitInfo.semaphore);
			waitSemaphores.push_back(timelineSemaphore->GetHandle());
			waitValues.push_back(waitInfo.value);
			waitStages.push_back(vk::PipelineStageFlagBits::eAllCommands); // TODO : Use an RHI equivalent
		}

		std::vector<vk::Semaphore> signalSemaphores;
		std::vector<size_t> signalValues;

		for (const SubmitInfo::SignalInfo signalInfo : _submitInfo.signalInfos)
		{
			auto* timelineSemaphore = dynamic_cast<TimelineSemaphore*>(signalInfo.semaphore);
			signalSemaphores.push_back(timelineSemaphore->GetHandle());
			signalValues.push_back(signalInfo.value);
		}

		vk::TimelineSemaphoreSubmitInfo timelineSemaphoreSubmitInfo;

		timelineSemaphoreSubmitInfo.setWaitSemaphoreValueCount(static_cast<uint32_t>(waitValues.size()));
		timelineSemaphoreSubmitInfo.setPWaitSemaphoreValues(waitValues.data());

		timelineSemaphoreSubmitInfo.setSignalSemaphoreValueCount(static_cast<uint32_t>(signalValues.size()));
		timelineSemaphoreSubmitInfo.setPSignalSemaphoreValues(signalValues.data());

		vk::SubmitInfo submitInfo;

		submitInfo.setPNext(&timelineSemaphoreSubmitInfo);

		submitInfo.setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));
		submitInfo.setPCommandBuffers(commandBuffers.data());

		submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()));
		submitInfo.setPWaitSemaphores(waitSemaphores.data());
		submitInfo.setPWaitDstStageMask(waitStages.data());

		submitInfo.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()));
		submitInfo.setPSignalSemaphores(signalSemaphores.data());
		submitInfo.setPNext(&timelineSemaphoreSubmitInfo);

		std::lock_guard<std::mutex> lock(submitMutex);
		queue.submit(submitInfo);
	}

	void Queue::WaitIdle()
	{
		std::lock_guard<std::mutex> lock(submitMutex);
		queue.waitIdle();
	}

	void Queue::SubmitBinaryToTimeline(
		const vk::Semaphore _binaryWait,
		const vk::Semaphore _timelineSignal,
		const uint64_t _timelineSignalValue
	)
	{
		constexpr uint64_t waitValue = 0;
		const uint64_t signalValue = _timelineSignalValue;

		vk::TimelineSemaphoreSubmitInfo timelineInfo;
		timelineInfo.setWaitSemaphoreValueCount(1);
		timelineInfo.setPWaitSemaphoreValues(&waitValue);
		timelineInfo.setSignalSemaphoreValueCount(1);
		timelineInfo.setPSignalSemaphoreValues(&signalValue);

		constexpr vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eTopOfPipe;

		vk::SubmitInfo submitInfo;
		submitInfo.setPNext(&timelineInfo);
		submitInfo.setWaitSemaphoreCount(1);
		submitInfo.setPWaitSemaphores(&_binaryWait);
		submitInfo.setPWaitDstStageMask(&waitStage);
		submitInfo.setSignalSemaphoreCount(1);
		submitInfo.setPSignalSemaphores(&_timelineSignal);

		std::lock_guard<std::mutex> lock(submitMutex);
		queue.submit(submitInfo);
	}

	void Queue::SubmitTimelineToBinary(
		const vk::Semaphore _timelineWait,
		const uint64_t _timelineWaitValue,
		const vk::Semaphore _binarySignal
	)
	{
		const uint64_t waitValue = _timelineWaitValue;
		constexpr uint64_t signalValue = 0;

		vk::TimelineSemaphoreSubmitInfo timelineInfo;
		timelineInfo.setWaitSemaphoreValueCount(1);
		timelineInfo.setPWaitSemaphoreValues(&waitValue);
		timelineInfo.setSignalSemaphoreValueCount(1);
		timelineInfo.setPSignalSemaphoreValues(&signalValue);

		constexpr vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;

		vk::SubmitInfo submitInfo;
		submitInfo.setPNext(&timelineInfo);
		submitInfo.setWaitSemaphoreCount(1);
		submitInfo.setPWaitSemaphores(&_timelineWait);
		submitInfo.setPWaitDstStageMask(&waitStage);
		submitInfo.setSignalSemaphoreCount(1);
		submitInfo.setPSignalSemaphores(&_binarySignal);

		std::lock_guard<std::mutex> lock(submitMutex);
		queue.submit(submitInfo);
	}

	vk::Result Queue::Present(const vk::PresentInfoKHR& _presentInfo)
	{
		std::lock_guard<std::mutex> lock(submitMutex);
		return queue.presentKHR(_presentInfo);
	}
}
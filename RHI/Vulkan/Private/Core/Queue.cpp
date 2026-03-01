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

	Queue::~Queue()
	{

	}

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

		queue.submit(submitInfo);
	}

	void Queue::WaitIdle()
	{
		queue.waitIdle();
	}
}
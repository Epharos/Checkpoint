#include "../pch.hpp"

#include "Queue.hpp"

#include <stdexcept>

namespace cp
{
	Queue::Queue(vk::Queue _queue, uint32_t _familyIndex, QueueType _type)
		: queue(_queue), familyIndex(_familyIndex), type(_type)
	{

	}

	Queue::~Queue()
	{

	}

	void Queue::Submit(const ISubmitInfo& _submitInfo)
	{
		// TODO : Implement this function to submit command buffers to the queue using the provided submit info
		throw std::logic_error("VulkanQueue::Submit is not implemented yet");
	}

	void Queue::WaitIdle()
	{
		queue.waitIdle();
	}

	QueueType Queue::GetType() const
	{
		return type;
	}

	vk::Queue& Queue::GetHandle()
	{
		return queue;
	}

	const vk::Queue& Queue::GetHandle() const
	{
		return queue;
	}

	uint32_t Queue::GetFamilyIndex() const
	{
		return familyIndex;
	}
}
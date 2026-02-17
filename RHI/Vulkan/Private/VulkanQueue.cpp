#include "pch.hpp"

#include "VulkanQueue.hpp"

#include <stdexcept>

namespace cp
{
	VulkanQueue::VulkanQueue(vk::Queue _queue, uint32_t _familyIndex, RHIQueueType _type)
		: queue(_queue), familyIndex(_familyIndex), type(_type)
	{

	}

	VulkanQueue::~VulkanQueue()
	{

	}

	void VulkanQueue::Submit(const RHISubmitInfo& _submitInfo)
	{
		// TODO : Implement this function to submit command buffers to the queue using the provided submit info
		throw std::logic_error("VulkanQueue::Submit is not implemented yet");
	}

	void VulkanQueue::WaitIdle()
	{
		queue.waitIdle();
	}

	RHIQueueType VulkanQueue::GetType() const
	{
		return type;
	}

	vk::Queue& VulkanQueue::GetHandle()
	{
		return queue;
	}

	const vk::Queue& VulkanQueue::GetHandle() const
	{
		return queue;
	}

	uint32_t VulkanQueue::GetFamilyIndex() const
	{
		return familyIndex;
	}
}
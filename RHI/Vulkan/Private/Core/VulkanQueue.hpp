#pragma once

#include "../pch.hpp"

#include <IQueue.hpp>

#include <optional>

namespace cp
{
	struct VulkanQueueFamilies
	{
#pragma push_macro("max")
#undef max
		uint32_t graphics = std::numeric_limits<uint32_t>::max();
		uint32_t compute = std::numeric_limits<uint32_t>::max();
		uint32_t transfer = std::numeric_limits<uint32_t>::max();
		uint32_t present = std::numeric_limits<uint32_t>::max();
#pragma pop_macro("max")
	};

	class VulkanQueue final : public IQueue
	{
	public:
		VulkanQueue(vk::Queue _queue, uint32_t _familyIndex, QueueType _type);
		~VulkanQueue() override;

		void Submit(const ISubmitInfo& _submitInfo) override;
		void WaitIdle() override;

		QueueType GetType() const override;
		
		vk::Queue& GetHandle();
		const vk::Queue& GetHandle() const;

		uint32_t GetFamilyIndex() const;

	private:
		vk::Queue queue;
		uint32_t familyIndex;
		QueueType type;
	};
}
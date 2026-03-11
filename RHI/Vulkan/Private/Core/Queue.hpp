#pragma once

#include "../pch.hpp"

#include <RHI/Core.hpp>

#include <optional>

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
		
		[[nodiscard]] vk::Queue& GetHandle() { return queue; }
		[[nodiscard]] const vk::Queue& GetHandle() const { return queue; }

		/**
		* @brief Returns the Vulkan queue family index this queue belongs to.
		*        Required when creating resources that must be used on a specific family (e.g. command pools).
		*
		* @return The queue family index.
		*/
		[[nodiscard]] uint32_t GetFamilyIndex() const { return familyIndex; }

	private:
		vk::Queue queue;
		uint32_t familyIndex;
		QueueType type;
	};
}
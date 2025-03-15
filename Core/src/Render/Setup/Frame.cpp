#include "pch.hpp"

#include "Frame.hpp"

namespace cp
{
	Frame::Frame(cp::VulkanContext*& _context) : context(_context)
	{
		vk::SemaphoreCreateInfo semaphoreInfo;

		imageAvailableSemaphore = context->GetDevice().createSemaphore(semaphoreInfo);
		renderFinishedSemaphore = context->GetDevice().createSemaphore(semaphoreInfo);

		vk::FenceCreateInfo fenceInfo;
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		inFlightFence = context->GetDevice().createFence(fenceInfo);

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.commandPool = context->GetCommandPool();
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		commandBuffer = context->GetDevice().allocateCommandBuffers(allocInfo)[0];
	}

	Frame::~Frame()
	{
		context->GetDevice().waitIdle();

		context->GetDevice().destroySemaphore(imageAvailableSemaphore);
		context->GetDevice().destroySemaphore(renderFinishedSemaphore);
		context->GetDevice().destroyFence(inFlightFence);

		context->GetDevice().freeCommandBuffers(context->GetCommandPool(), commandBuffer);

		for (auto& renderTarget : renderTargets)
		{
			delete renderTarget;
		}
	}

	void Frame::AddRenderTarget(RenderTarget* _renderTarget)
	{
		renderTargets.push_back(_renderTarget);
	}
}
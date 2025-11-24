#include "pch.hpp"
#include "RendererPrototype.hpp"

#include "RendererInstance.hpp"

#include "../Setup/Frame.hpp"

void cp::RendererPrototype::CreateFixedPipelines(RendererInstance& _instance) {}
void cp::RendererPrototype::CreateRenderPasses(RendererInstance& _instance) {}

uint32_t cp::RendererPrototype::PrepareFrame(cp::Swapchain* _swapchain)
{
	if (!_swapchain)
	{
		LOG_ERROR("Swapchain is not initialized, cannot prepare frame");
		return ~0u;
	}

	if (_swapchain->GetCurrentFrameIndex() >= _swapchain->GetFrameCount())
	{
		LOG_WARNING("Current frame index is out of bounds, resetting to 0");
		_swapchain->SetCurrentFrame(0);
		return -1;
	}

	if (context->GetDevice().waitForFences(1, &_swapchain->GetCurrentFrame()->GetInFlightFence(), VK_TRUE, std::numeric_limits<uint32_t>().max()) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to wait for fence");
	}

	vk::Result result = context->GetDevice().resetFences(1, &_swapchain->GetCurrentFrame()->GetInFlightFence());

	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to reset fence");
	}

	uint32 imageIndex;

	try
	{
		imageIndex = context->GetDevice().acquireNextImageKHR(_swapchain->GetSwapchain(), std::numeric_limits<uint64_t>().max(), _swapchain->GetCurrentFrame()->GetImageAvailableSemaphore(), nullptr).value;
	}
	catch (vk::OutOfDateKHRError e)
	{
		_swapchain->Recreate();
		return -1;
	}

	_swapchain->GetCurrentFrame()->GetCommandBuffer().reset();

	vk::CommandBufferBeginInfo beginInfo = {};
	_swapchain->GetCurrentFrame()->GetCommandBuffer().begin(beginInfo);

	return imageIndex;
}

void cp::RendererPrototype::SubmitFrame(cp::Swapchain* _swapchain)
{
	_swapchain->GetCurrentFrame()->GetCommandBuffer().end();

	vk::SubmitInfo submitInfo = {};
	vk::Semaphore waitSemaphores[] = { _swapchain->GetCurrentFrame()->GetImageAvailableSemaphore() };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	vk::Semaphore signalSemaphores[] = { _swapchain->GetCurrentFrame()->GetRenderFinishedSemaphore() };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_swapchain->GetCurrentFrame()->GetCommandBuffer();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vk::Queue graphicsQueue = context->GetDevice().getQueue(context->GetQueueFamilyIndices().graphicsFamily.value(), 0);

	vk::Result result = graphicsQueue.submit(1, &submitInfo, _swapchain->GetCurrentFrame()->GetInFlightFence());

	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to submit command buffer");
	}
}

void cp::RendererPrototype::PresentFrame(cp::Swapchain* _swapchain, uint32_t _index)
{
	vk::Result result = vk::Result::eSuccess;
	vk::Queue graphicsQueue = context->GetDevice().getQueue(context->GetQueueFamilyIndices().graphicsFamily.value(), 0);
	vk::Semaphore signalSemaphores[] = { _swapchain->GetCurrentFrame()->GetRenderFinishedSemaphore() };

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	vk::SwapchainKHR swapchains[] = { _swapchain->GetSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &_index;

	try
	{
		result = graphicsQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError e)
	{
		_swapchain->Recreate();
	}

	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
	{
		_swapchain->Recreate();
	}
	else if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present swapchain image");
	}
}

void cp::RendererPrototype::EndFrame(cp::Swapchain* _swapchain)
{
	_swapchain->SetCurrentFrame((_swapchain->GetCurrentFrameIndex() + 1) % _swapchain->GetFrameCount());
}

cp::RendererPrototype::RendererPrototype(cp::VulkanContext* _context) : context(_context) {}

cp::RendererPrototype::~RendererPrototype() {}

void cp::RendererPrototype::BuildForInstance(RendererInstance& _instance)
{
	CreateFixedPipelines(_instance);
	CreateMainRenderPass(_instance);
	CreateRenderPasses(_instance);
}

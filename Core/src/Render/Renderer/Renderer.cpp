#include "pch.hpp"
#include "Renderer.hpp"

#include "../Setup/Frame.hpp"

void Render::Renderer::CreateRenderPasses()
{

}

uint32_t Render::Renderer::PrepareFrame()
{
	if (context->GetDevice().waitForFences(1, &swapchain->GetCurrentFrame()->GetInFlightFence(), VK_TRUE, std::numeric_limits<uint32_t>().max()) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to wait for fence");
	}

	uint32 imageIndex;

	try
	{
		imageIndex = context->GetDevice().acquireNextImageKHR(swapchain->GetSwapchain(), std::numeric_limits<uint64_t>().max(), swapchain->GetCurrentFrame()->GetImageAvailableSemaphore(), nullptr).value;
	}
	catch (vk::OutOfDateKHRError e)
	{
		swapchain->Recreate();
		return -1;
	}

	swapchain->GetCurrentFrame()->GetCommandBuffer().reset();

	vk::CommandBufferBeginInfo beginInfo = {};
	swapchain->GetCurrentFrame()->GetCommandBuffer().begin(beginInfo);

	return imageIndex;
}

void Render::Renderer::SubmitFrame()
{
	swapchain->GetCurrentFrame()->GetCommandBuffer().end();

	vk::SubmitInfo submitInfo = {};
	vk::Semaphore waitSemaphores[] = { swapchain->GetCurrentFrame()->GetImageAvailableSemaphore() };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	vk::Semaphore signalSemaphores[] = { swapchain->GetCurrentFrame()->GetRenderFinishedSemaphore() };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &swapchain->GetCurrentFrame()->GetCommandBuffer();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vk::Result result = context->GetDevice().resetFences(1, &swapchain->GetCurrentFrame()->GetInFlightFence());

	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to reset fence");
	}

	vk::Queue graphicsQueue = context->GetDevice().getQueue(context->GetQueueFamilyIndices().graphicsFamily.value(), 0);

	result = graphicsQueue.submit(1, &submitInfo, swapchain->GetCurrentFrame()->GetInFlightFence());

	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to submit command buffer");
	}
}

void Render::Renderer::PresentFrame(uint32_t _index)
{
	vk::Result result;
	vk::Queue graphicsQueue = context->GetDevice().getQueue(context->GetQueueFamilyIndices().graphicsFamily.value(), 0);
	vk::Semaphore signalSemaphores[] = { swapchain->GetCurrentFrame()->GetRenderFinishedSemaphore() };

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	vk::SwapchainKHR swapchains[] = { swapchain->GetSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &_index;

	try
	{
		result = graphicsQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError e)
	{
		swapchain->Recreate();
	}

	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
	{
		swapchain->Recreate();
	}
	else if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present swapchain image");
	}
}

void Render::Renderer::EndFrame()
{
	swapchain->SetCurrentFrame((swapchain->GetCurrentFrameIndex() + 1) % swapchain->GetFrameCount());
}

void Render::Renderer::Build(Context::VulkanContext* _context)
{
	context = _context;
	swapchain = new Swapchain(_context);
	CreateMainRenderPass();
	CreateRenderPasses();
	swapchain->Create(mainRenderPass);
	mainCamera = new Camera(context);

	pipelinesManager = new Pipeline::PipelinesManager(context->GetDevice());
	layoutsManager = new Pipeline::LayoutsManager(context->GetDevice());
	descriptorSetLayoutsManager = new Pipeline::DescriptorSetLayoutsManager(context->GetDevice());
	descriptorSetManager = new Pipeline::DescriptorSetManager(context->GetDevice());

	SetupPipelines();
}

Render::Renderer::~Renderer()
{
	
}

void Render::Renderer::Cleanup()
{
	delete swapchain;
	context->GetDevice().destroyRenderPass(mainRenderPass);

	pipelinesManager->Cleanup();
	layoutsManager->Cleanup();
	descriptorSetLayoutsManager->Cleanup();
	descriptorSetManager->Cleanup();

	delete mainCamera;
	delete pipelinesManager;
	delete layoutsManager;
	delete descriptorSetLayoutsManager;
	delete descriptorSetManager;
}

void Render::Renderer::Render(const std::vector<InstanceGroup>& _instanceGroups)
{
	uint32 index = PrepareFrame();

	RenderFrame(_instanceGroups);

	SubmitFrame();
	PresentFrame(index);

	EndFrame();
}

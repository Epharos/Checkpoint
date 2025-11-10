#include "pch.hpp"
#include "Renderer.hpp"

#include "../Setup/Frame.hpp"

void cp::Renderer::SetupSurface(Platform* _platform)
{
	VkSurfaceKHR surfaceHandle = VK_NULL_HANDLE;
	VkResult vr = VkResult::VK_RESULT_MAX_ENUM; // Just to silence uninitialized variable warning

	switch (_platform->GetType())
	{
	case PlatformType::GLFW:
		vr = glfwCreateWindowSurface(context->GetInstance(), (GLFWwindow*)_platform->GetNativeWindowHandle(), nullptr, &surfaceHandle);

		if (vr != VK_SUCCESS)
		{
			LOG_FATAL("Failed to create window surface " + vr);
			return;
		}

		LOG_INFO("Successfully created GLFW window surface");
		break;
	case PlatformType::QT:
		LOG_WARNING("Qt platform surface is handled externally, skipping surface creation. Use SetSurface(VkSurfaceKHR) once it's set up.");
		break;
	default:
		LOG_FATAL("Unsupported platform type for surface creation");
		return;
	}

	surface = surfaceHandle;
	platform = _platform;
}

void cp::Renderer::CreateRenderPasses()
{

}

uint32_t cp::Renderer::PrepareFrame()
{
	if (!swapchain)
	{
		LOG_ERROR("Swapchain is not initialized, cannot prepare frame");
		return ~0u;
	}

	if(swapchain->GetCurrentFrameIndex() >= swapchain->GetFrameCount())
	{
		LOG_WARNING("Current frame index is out of bounds, resetting to 0");
		swapchain->SetCurrentFrame(0);
		return -1;
	}

	if (context->GetDevice().waitForFences(1, &swapchain->GetCurrentFrame()->GetInFlightFence(), VK_TRUE, std::numeric_limits<uint32_t>().max()) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to wait for fence");
	}

	vk::Result result = context->GetDevice().resetFences(1, &swapchain->GetCurrentFrame()->GetInFlightFence());

	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to reset fence");
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

void cp::Renderer::SubmitFrame()
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

	vk::Queue graphicsQueue = context->GetDevice().getQueue(context->GetQueueFamilyIndices().graphicsFamily.value(), 0);

	vk::Result result = graphicsQueue.submit(1, &submitInfo, swapchain->GetCurrentFrame()->GetInFlightFence());

	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to submit command buffer");
	}
}

void cp::Renderer::PresentFrame(uint32_t _index)
{
	vk::Result result = vk::Result::eSuccess;
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

void cp::Renderer::EndFrame()
{
	swapchain->SetCurrentFrame((swapchain->GetCurrentFrameIndex() + 1) % swapchain->GetFrameCount());
}

cp::Renderpass& cp::Renderer::RegisterRenderPass(const std::string& _name)
{
	if (renderPasses.find(_name) == renderPasses.end())
	{
		renderPasses.insert({ _name, cp::Renderpass(context, _name) });
	}

	return renderPasses.at(_name);
}

cp::Renderpass& cp::Renderer::GetRenderPass(const std::string& _name)
{
	return renderPasses.at(_name);
}

void cp::Renderer::SetPlatform(Platform* _platform)
{
	platform = _platform;
	SetupSurface(_platform);
}

std::vector<std::string> cp::Renderer::GetRenderPassNames()
{
	std::vector<std::string> names;

	for (auto& [name, id] : renderPasses)
	{
		names.push_back(name);
	}

	return names;
}

void cp::Renderer::Build()
{
	swapchain = new Swapchain(context, surface, platform);
	CreateMainRenderPass();
	CreateRenderPasses();
	swapchain->Create(mainRenderPass);

	SetupPipelines();
}

cp::Renderer::Renderer(cp::VulkanContext* _context) : context(_context)
{
	RegisterRenderPass("Main");
}

cp::Renderer::~Renderer()
{
	
}

void cp::Renderer::Cleanup()
{
	delete swapchain;
	context->GetDevice().destroyRenderPass(mainRenderPass);
	context->GetInstance().destroySurfaceKHR(surface);
}

void cp::Renderer::Render(const std::vector<InstanceGroup>& _instanceGroups)
{
	uint32 index = PrepareFrame();

	RenderFrame(_instanceGroups);

	SubmitFrame();
	PresentFrame(index);

	EndFrame();
}
#include "RendererInstance.hpp"

cp::RendererInstance::RendererInstance(cp::VulkanContext* _context, Platform* _platform, RendererPrototype* _prototype)
	: context(_context), platform(_platform), prototype(_prototype)
{
	LOG_DEBUG("Constructing RendererInstance");

	if (!_prototype)
	{
		LOG_FATAL("RendererPrototype is null in RendererInstance constructor");
		throw std::runtime_error("RendererPrototype is null");
	}

	_context->GetDevice().waitIdle();

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

	swapchain = new Swapchain(context, surface, platform);
	swapchain->Create(prototype->GetRenderPasses().at("Main").GetRenderPass()); // Use the prototype's main render pass
}

cp::RendererInstance::~RendererInstance()
{
	context->GetDevice().waitIdle();

	if (swapchain)
	{
		delete swapchain;
	}
}

void cp::RendererInstance::TriggerSwapchainRecreation()
{
	if (swapchain) swapchain->Recreate();
}

void cp::RendererInstance::Render(const std::vector<InstanceGroup>& _instanceGroups)
{
	if (prototype) prototype->Render(_instanceGroups);
}

void cp::RendererInstance::SetSurface(vk::SurfaceKHR _surface)
{
	if (platform->GetType() != PlatformType::QT) {
		LOG_ERROR("SetSurface is intended for use with the Qt platform. Current platform type does not require external surface setting.");
		return;
	}

	surface = _surface;
}

void cp::RendererInstance::ResetSwapchain()
{
	if (swapchain)
	{
		delete swapchain;
		swapchain = new Swapchain(context, surface, platform);
		swapchain->Create(prototype->GetRenderPasses().at("Main").GetRenderPass()); // Use the prototype's main render pass
	}
}

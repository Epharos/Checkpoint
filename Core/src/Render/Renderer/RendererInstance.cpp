#include "RendererInstance.hpp"
#include "Render/Renderer/Camera.hpp"

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

	instancedBuffer = Helper::Memory::CreateBuffer(
		_context->GetDevice(),
		_context->GetPhysicalDevice(),
		sizeof(cp::TransformData) * MAX_RENDERABLE_ENTITIES,
		vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	cp::DescriptorSetUpdate instanceBufferUpdate;
	instanceBufferUpdate.updateType = cp::DescriptorSetUpdateType::BUFFER;
	instanceBufferUpdate.buffer = instancedBuffer.buffer;
	instanceBufferUpdate.offset = 0;
	instanceBufferUpdate.range = sizeof(cp::TransformData) * MAX_RENDERABLE_ENTITIES;
	instanceBufferUpdate.dstBinding = 0;
	instanceBufferUpdate.dstArrayElement = 0;
	instanceBufferUpdate.descriptorType = vk::DescriptorType::eStorageBuffer;
	instanceBufferUpdate.descriptorCount = 1;
	context->GetDescriptorSetManager()->UpdateDescriptorSet("Instanced Drawing", { instanceBufferUpdate });

	cameraBuffer = Helper::Memory::CreateBuffer(
		_context->GetDevice(),
		_context->GetPhysicalDevice(),
		sizeof(cp::CameraUBO),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	cp::DescriptorSetUpdate cameraBufferUpdate;
	cameraBufferUpdate.updateType = cp::DescriptorSetUpdateType::BUFFER;
	cameraBufferUpdate.buffer = cameraBuffer.buffer;
	cameraBufferUpdate.offset = 0;
	cameraBufferUpdate.range = sizeof(cp::CameraUBO);
	cameraBufferUpdate.dstBinding = 0;
	cameraBufferUpdate.dstArrayElement = 0;
	cameraBufferUpdate.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraBufferUpdate.descriptorCount = 1;
	context->GetDescriptorSetManager()->UpdateDescriptorSet("Global Unlit", { cameraBufferUpdate });

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

	if (!surface)
	{
		LOG_WARNING("Surface is null after creation in RendererInstance constructor");
		return;
	}

	swapchain = new Swapchain(context, surface, platform);
	swapchain->Create(); // Use the prototype's main render pass
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
	if (prototype) prototype->Render(this, _instanceGroups);
}

void cp::RendererInstance::SetSurface(vk::SurfaceKHR _surface)
{
	if (platform->GetType() != PlatformType::QT) {
		LOG_ERROR("SetSurface is intended for use with the Qt platform. Current platform type does not require external surface setting.");
		return;
	}

	surface = _surface;
	swapchain = new Swapchain(context, surface, platform);
	swapchain->Create();
}

void cp::RendererInstance::ResetSwapchain()
{
	if (swapchain)
	{
		delete swapchain;
		swapchain = new Swapchain(context, surface, platform);
		swapchain->Create(); // Use the prototype's main render pass
	}
}

void cp::RendererInstance::UpdateCameraBuffer(cp::Buffer _buffer)
{
	cp::DescriptorSetUpdate cameraBufferUpdate;
	cameraBufferUpdate.updateType = cp::DescriptorSetUpdateType::BUFFER;
	cameraBufferUpdate.buffer = _buffer.buffer;
	cameraBufferUpdate.offset = 0;
	cameraBufferUpdate.range = sizeof(cp::CameraUBO);
	cameraBufferUpdate.dstBinding = 0;
	cameraBufferUpdate.dstArrayElement = 0;
	cameraBufferUpdate.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraBufferUpdate.descriptorCount = 1;
	context->GetDescriptorSetManager()->UpdateDescriptorSet("Global Unlit", { cameraBufferUpdate });
}
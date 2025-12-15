#pragma once

#include "../../pch.hpp"
#include "../../Context/VulkanContext.hpp"
#include "../Setup/Swapchain.hpp"

#include "RendererPrototype.hpp"

template<>
class std::hash<cp::RenderpassDescription>
{
	public:
	size_t operator()(const cp::RenderpassDescription& _desc) const
	{
		return std::hash<std::string>()(_desc.GetName());
	}
};

namespace cp {
	class RendererInstance
	{
	private:
		cp::VulkanContext* context = nullptr;
		Platform* platform = nullptr;
		vk::SurfaceKHR surface = VK_NULL_HANDLE;
		Swapchain* swapchain = nullptr;

		RendererPrototype* prototype = nullptr;

		constexpr static size_t MAX_RENDERABLE_ENTITIES = 10000;

		cp::Buffer cameraBuffer;
		cp::Buffer instancedBuffer;

	public:
		RendererInstance(cp::VulkanContext* _context, Platform* _platform, RendererPrototype* _prototype);
		~RendererInstance();

		void TriggerSwapchainRecreation();

		void Render(const std::vector<InstanceGroup>& _instanceGroups);

		Platform* GetPlatform() const { return platform; }
		Swapchain* GetSwapchain() const { return swapchain; }
		vk::SurfaceKHR GetSurface() const { return surface; }
		cp::VulkanContext* GetContext() const { return context; }
		RendererPrototype* GetPrototype() const { return prototype; }

		void SetSurface(vk::SurfaceKHR _surface);
		void ResetSwapchain();

		cp::Buffer& GetCameraBuffer() { return cameraBuffer; }
		cp::Buffer& GetInstancedBuffer() { return instancedBuffer; }

		void UpdateCameraBuffer(cp::Buffer _buffer);
	};
}
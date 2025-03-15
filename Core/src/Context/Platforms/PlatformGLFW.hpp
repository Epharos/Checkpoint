#pragma once

#include "../../pch.hpp"
#include "Platform.hpp"

namespace cp
{
	struct VulkanContextInfo;

	class PlatformGLFW : public Platform
	{
	private:
		GLFWwindow* window = nullptr;

	public:
		PlatformGLFW() : Platform(GLFW) {}

		void Initialize(VulkanContextInfo _context, vk::Extent2D _extent = vk::Extent2D(1920, 1080));
		bool ShouldClose() const;
		void PollEvents() const;
		void CleanUp() const;

		void SetTitle(const std::string& _title) const;

		inline constexpr void* GetNativeWindowHandle() const override { return window; }
		vk::Extent2D GetExtent() const;
	};
}
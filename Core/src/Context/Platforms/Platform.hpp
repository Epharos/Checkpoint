#pragma once

#include "../../pch.hpp"

namespace Context
{
	struct VulkanContextInfo;

	enum PlatformType
	{
		GLFW,
		QT
	};

	class Platform
	{	
	protected:
		PlatformType type;

		Platform(PlatformType _type) : type(_type) {}
	public:
		virtual void Initialize(VulkanContextInfo _context, vk::Extent2D _extent = vk::Extent2D(1920, 1080)) = 0;
		virtual bool ShouldClose() const = 0;
		virtual void PollEvents() const = 0;
		virtual void CleanUp() const = 0;

		virtual void SetTitle(const std::string& _title) const = 0;

		virtual constexpr void* GetNativeWindowHandle() const = 0;
		virtual vk::Extent2D GetExtent() const = 0;

		inline constexpr float GetAspectRatio() const { return static_cast<float>(GetExtent().width) / static_cast<float>(GetExtent().height); }

		inline constexpr PlatformType GetType() const { return type; }
	};
}
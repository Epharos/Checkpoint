#pragma once

#include "../../pch.hpp"
#include "Platform.hpp"
#include <QtGui/qwindow.h>

namespace cp
{
	struct VulkanContextInfo;

	class PlatformQt : public Platform
	{
	private:
		QWindow* window = nullptr;

	public:
		PlatformQt() : Platform(QT) {}

		void Initialize(VulkanContextInfo _context, vk::Extent2D _extent = vk::Extent2D(1920, 1080));
		void Initialize(QWindow* _window);
		bool ShouldClose() const override;
		void PollEvents() const override;
		void CleanUp() const override;

		void SetTitle(const std::string& _title) const override;

		inline constexpr void* GetNativeWindowHandle() const override { return window; }
		vk::Extent2D GetExtent() const override;
	};
}
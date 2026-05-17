#pragma once

#include <string_view>
#include <string>
#include <cstdint>
#include <functional>

#include <../../Common/Public/Common/Data/Extent.hpp>

namespace cp
{
	struct WindowInfo
	{
		std::string title = "Checkpoint";

		Extent2D<int> extent{ 1280, 720 };

		bool resizable = true;
		bool fullscreen = false;
		bool vsync = true;
	};

	class IWindow
	{
	public:
		IWindow(const WindowInfo& _info);
		virtual ~IWindow() = default;

		virtual void* GetNativeWindowHandle() const = 0;
		virtual float GetAspectRatio() const = 0;

		virtual void PollEvents() const = 0;
		virtual void WaitForEvents() const = 0;
		virtual bool ShouldClose() const = 0;

		virtual void SetTitle(std::string_view _title) const = 0;
		virtual std::string_view GetTitle() const = 0;

		virtual void SetExtent(Extent2D<int> _extent) = 0;
		virtual Extent2D<int> GetExtent() const = 0;

		virtual void SetResizable(bool _resizable) = 0;
		virtual bool IsResizable() const = 0;

		virtual void SetFullscreen(bool _fullscreen) = 0;
		virtual bool IsFullscreen() const = 0;

		virtual void SetVSyncActive(bool _vsync) = 0;
		virtual bool IsVSyncActive() const = 0;

		virtual void CleanUp() const = 0;

		void SetResizeCallback(std::function<void(Extent2D<int>)> _callback);

	protected:
		void InvokeResizeCallback(Extent2D<int> _newExtent) const;

	private:
		std::function<void(Extent2D<int>)> resizeCallback;
	};
}
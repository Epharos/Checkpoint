#pragma once

#include <string_view>
#include <string>
#include <cstdint>

namespace cp
{
	struct WindowInfo
	{
		std::string title = "Checkpoint";

		uint32_t width = 1920;
		uint32_t height = 1080;
		// TODO : Change uint32_t to an extent struct

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
		virtual bool ShouldClose() const = 0;

		virtual void SetTitle(std::string_view _title) const = 0;
		virtual std::string_view GetTitle() const = 0;

		virtual void SetExtent(uint32_t _width, uint32_t _height) = 0; // TODO : Change uint32_t to an extent struct
		virtual void GetExtent(uint32_t& _width, uint32_t& _height) const = 0; // TODO : Change uint32_t to an extent struct

		virtual void SetResizable(bool _resizable) = 0;
		virtual bool IsResizable() const = 0;

		virtual void SetFullscreen(bool _fullscreen) = 0;
		virtual bool IsFullscreen() const = 0;

		virtual void SetVSyncActive(bool _vsync) = 0;
		virtual bool IsVSyncActive() const = 0;

		virtual void CleanUp() const = 0;
	};
}
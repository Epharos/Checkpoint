#pragma once

#include <IWindow.hpp>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <Windows.h>

#include <unordered_set>

namespace cp
{
	class NativeWindow : public IWindow
	{
	public:
		NativeWindow(const WindowInfo& _info);
		~NativeWindow() override;

		NativeWindow(const NativeWindow&) = delete;
		NativeWindow& operator=(const NativeWindow&) = delete;
		NativeWindow(NativeWindow&&) = delete;
		NativeWindow& operator=(NativeWindow&&) = delete;

		void* GetNativeWindowHandle() const override;
		float GetAspectRatio() const override;

		void PollEvents() const override;
		void WaitForEvents() const override;
		bool ShouldClose() const override;

		void SetTitle(std::string_view _title) const override;
		std::string_view GetTitle() const override;

		void SetExtent(Extent2D<int> _extent) override;
		Extent2D<int> GetExtent() const override;

		void SetResizable(bool _resizable) override;
		bool IsResizable() const override;

		void SetFullscreen(bool _fullscreen) override;
		bool IsFullscreen() const override;

		void SetVSyncActive(bool _vsync) override;
		bool IsVSyncActive() const override;

		void CleanUp() const override;
		void FillRawInputState(RawInputState& _state) const override;

	private:
		static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wp, LPARAM _lp);

		mutable HWND hwnd = nullptr;
		mutable ATOM classAtom = 0;
		mutable bool closeRequested = false;
		mutable std::string title;

		bool fullscreen = false;
		bool resizable = true;
		bool vsyncActive = false;

		WINDOWPLACEMENT savedPlacement{ sizeof(WINDOWPLACEMENT) };

		mutable std::unordered_set<Key> pressedKeys;
		mutable glm::vec2 mousePosition { 0.f, 0.f };
		mutable glm::vec2 mouseScroll { 0.f, 0.f };
		mutable uint32_t mouseButtons = 0;
	};
}

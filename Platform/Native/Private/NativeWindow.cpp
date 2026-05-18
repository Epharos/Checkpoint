#include "NativeWindow.hpp"

#include <windowsx.h>

namespace
{
	constexpr wchar_t NativeWindowClassName[] = L"CheckpointNativeWindow";

	std::wstring Utf8ToWide(std::string_view _str)
	{
		if (_str.empty())
		{
			return {};
		}

		const int size = MultiByteToWideChar(CP_UTF8, 0, _str.data(), static_cast<int>(_str.size()), nullptr, 0);
		std::wstring result(size, 0);
		MultiByteToWideChar(CP_UTF8, 0, _str.data(), static_cast<int>(_str.size()), result.data(), size);
		return result;
	}

	cp::Key VkToKey(const UINT _vk)
	{
		if (_vk >= 'A' && _vk <= 'Z') return static_cast<cp::Key>(static_cast<uint16_t>(cp::Key::A) + (_vk - 'A'));
		if (_vk >= '0' && _vk <= '9') return static_cast<cp::Key>(static_cast<uint16_t>(cp::Key::D0) + (_vk - '0'));
		if (_vk >= VK_F1 && _vk <= VK_F12) return static_cast<cp::Key>(static_cast<uint16_t>(cp::Key::F1) + (_vk - VK_F1));
		if (_vk >= VK_NUMPAD0 && _vk <= VK_NUMPAD9) return static_cast<cp::Key>(static_cast<uint16_t>(cp::Key::Numpad0) + (_vk - VK_NUMPAD0));

		switch (_vk)
		{
		case VK_SPACE: return cp::Key::Space;
		case VK_RETURN: return cp::Key::Enter;
		case VK_ESCAPE: return cp::Key::Escape;
		case VK_TAB: return cp::Key::Tab;
		case VK_BACK: return cp::Key::Backspace;
		case VK_DELETE: return cp::Key::Delete;
		case VK_INSERT: return cp::Key::Insert;
		case VK_HOME: return cp::Key::Home;
		case VK_END: return cp::Key::End;
		case VK_PRIOR: return cp::Key::PageUp;
		case VK_NEXT: return cp::Key::PageDown;
		case VK_LEFT: return cp::Key::Left;
		case VK_RIGHT: return cp::Key::Right;
		case VK_UP: return cp::Key::Up;
		case VK_DOWN: return cp::Key::Down;
		case VK_LSHIFT: return cp::Key::LeftShift;
		case VK_RSHIFT: return cp::Key::RightShift;
		case VK_LCONTROL: return cp::Key::LeftCtrl;
		case VK_RCONTROL: return cp::Key::RightCtrl;
		case VK_LMENU: return cp::Key::LeftAlt;
		case VK_RMENU: return cp::Key::RightAlt;
		case VK_LWIN: return cp::Key::LeftSuper;
		case VK_RWIN: return cp::Key::RightSuper;
		case VK_ADD: return cp::Key::NumpadAdd;
		case VK_SUBTRACT: return cp::Key::NumpadSubtract;
		case VK_MULTIPLY: return cp::Key::NumpadMultiply;
		case VK_DIVIDE: return cp::Key::NumpadDivide;
		case VK_DECIMAL: return cp::Key::NumpadDecimal;
		case VK_OEM_MINUS: return cp::Key::Minus;
		case VK_OEM_PLUS: return cp::Key::Equal;
		case VK_OEM_4: return cp::Key::LeftBracket;
		case VK_OEM_6: return cp::Key::RightBracket;
		case VK_OEM_5: return cp::Key::Backslash;
		case VK_OEM_1: return cp::Key::Semicolon;
		case VK_OEM_7: return cp::Key::Apostrophe;
		case VK_OEM_3: return cp::Key::GraveAccent;
		case VK_OEM_COMMA: return cp::Key::Comma;
		case VK_OEM_PERIOD: return cp::Key::Period;
		case VK_OEM_2: return cp::Key::Slash;
		case VK_CAPITAL: return cp::Key::CapsLock;
		case VK_NUMLOCK: return cp::Key::NumLock;
		case VK_SCROLL: return cp::Key::ScrollLock;
		case VK_SNAPSHOT: return cp::Key::PrintScreen;
		case VK_PAUSE: return cp::Key::Pause;
		default: return cp::Key::Unknown;
		}
	}
}

namespace cp
{
	LRESULT CALLBACK NativeWindow::WindowProc(HWND _hwnd, UINT _msg, WPARAM _wp, LPARAM _lp)
	{
		if (_msg == WM_NCCREATE)
		{
			const auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(_lp);
			SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
		}

		auto* self = reinterpret_cast<NativeWindow*>(GetWindowLongPtrW(_hwnd, GWLP_USERDATA));

		switch (_msg)
		{
		case WM_CLOSE:
			if (self != nullptr)
			{
				self->closeRequested = true;
			}
			return 0;

		case WM_SIZE:
			if (self != nullptr && _wp != SIZE_MINIMIZED)
			{
				const int width  = LOWORD(_lp);
				const int height = HIWORD(_lp);

				if (width > 0 && height > 0)
				{
					self->InvokeResizeCallback(Extent2D<int>{ width, height });
				}
			}
			return DefWindowProcW(_hwnd, _msg, _wp, _lp);

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (self != nullptr)
			{
				const Key key = VkToKey(static_cast<UINT>(_wp));
				if (key != Key::Unknown)
				{
					self->pressedKeys.insert(key);
				}
			}

			return DefWindowProcW(_hwnd, _msg, _wp, _lp);

		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (self != nullptr)
			{
				const Key key = VkToKey(static_cast<UINT>(_wp));
				if (key != Key::Unknown)
				{
					self->pressedKeys.erase(key);
				}
			}

			return DefWindowProcW(_hwnd, _msg, _wp, _lp);

		// ---- mouse movement / scroll ----------------------------------------
		case WM_MOUSEMOVE:
			if (self != nullptr)
			{
				self->mousePosition = {
					static_cast<float>(GET_X_LPARAM(_lp)),
					static_cast<float>(GET_Y_LPARAM(_lp))
				};
			}

			return DefWindowProcW(_hwnd, _msg, _wp, _lp);

		case WM_MOUSEWHEEL:
			if (self != nullptr)
			{
				self->mouseScroll.y += static_cast<float>(GET_WHEEL_DELTA_WPARAM(_wp)) / static_cast<float>(WHEEL_DELTA);
			}

			return DefWindowProcW(_hwnd, _msg, _wp, _lp);

		case WM_MOUSEHWHEEL:
			if (self != nullptr)
			{
				self->mouseScroll.x += static_cast<float>(GET_WHEEL_DELTA_WPARAM(_wp)) / static_cast<float>(WHEEL_DELTA);
			}

			return DefWindowProcW(_hwnd, _msg, _wp, _lp);

		case WM_LBUTTONDOWN: if (self) self->mouseButtons |=  (1u << 0); return 0;
		case WM_LBUTTONUP: if (self) self->mouseButtons &= ~(1u << 0); return 0;
		case WM_RBUTTONDOWN: if (self) self->mouseButtons |=  (1u << 1); return 0;
		case WM_RBUTTONUP: if (self) self->mouseButtons &= ~(1u << 1); return 0;
		case WM_MBUTTONDOWN: if (self) self->mouseButtons |=  (1u << 2); return 0;
		case WM_MBUTTONUP: if (self) self->mouseButtons &= ~(1u << 2); return 0;

		case WM_XBUTTONDOWN:
			if (self)
			{
				const WORD xBtn = GET_XBUTTON_WPARAM(_wp);
				if (xBtn == XBUTTON1) self->mouseButtons |= (1u << 3);
				else self->mouseButtons |= (1u << 4);
			}

			return TRUE;

		case WM_XBUTTONUP:
			if (self)
			{
				const WORD xBtn = GET_XBUTTON_WPARAM(_wp);
				if (xBtn == XBUTTON1) self->mouseButtons &= ~(1u << 3);
				else self->mouseButtons &= ~(1u << 4);
			}

			return TRUE;

		case WM_KILLFOCUS:
			if (self != nullptr)
			{
				self->pressedKeys.clear();
				self->mouseButtons = 0;
			}

			return DefWindowProcW(_hwnd, _msg, _wp, _lp);

		default:
			return DefWindowProcW(_hwnd, _msg, _wp, _lp);
		}
	}

	NativeWindow::NativeWindow(const WindowInfo& _info) :
		IWindow(_info), title(_info.title), resizable(_info.resizable), vsyncActive(_info.vsync)
	{
		const HINSTANCE hInstance = GetModuleHandleW(nullptr);

		WNDCLASSEXW wc = {};
		wc.cbSize = sizeof(WNDCLASSEXW);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
		wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wc.lpszClassName = NativeWindowClassName;

		classAtom = RegisterClassExW(&wc);
		if (classAtom == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		{
			return;
		}

		DWORD style = WS_OVERLAPPEDWINDOW;
		if (!_info.resizable)
		{
			style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		}

		RECT rect = { 0, 0, _info.extent.x(), _info.extent.y() };
		AdjustWindowRect(&rect, style, FALSE);

		const std::wstring wideTitle = Utf8ToWide(_info.title);
		hwnd = CreateWindowExW(
			0,
			NativeWindowClassName,
			wideTitle.c_str(),
			style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left,
			rect.bottom - rect.top,
			nullptr, nullptr, hInstance, this
		);

		if (hwnd != nullptr)
		{
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
		}
	}

	NativeWindow::~NativeWindow()
	{
		CleanUp();
	}

	void NativeWindow::CleanUp() const
	{
		if (hwnd != nullptr)
		{
			DestroyWindow(hwnd);
			hwnd = nullptr;
		}

		if (classAtom != 0)
		{
			UnregisterClassW(NativeWindowClassName, GetModuleHandleW(nullptr));
			classAtom = 0;
		}
	}

	void* NativeWindow::GetNativeWindowHandle() const
	{
		return hwnd;
	}

	float NativeWindow::GetAspectRatio() const
	{
		const auto extent = GetExtent();

		if (extent.y() == 0)
		{
			return 1.0f;
		}

		return static_cast<float>(extent.x()) / static_cast<float>(extent.y());
	}

	void NativeWindow::PollEvents() const
	{
		MSG msg;
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	void NativeWindow::WaitForEvents() const
	{
		WaitMessage();
		PollEvents();
	}

	bool NativeWindow::ShouldClose() const
	{
		return closeRequested || hwnd == nullptr || !IsWindow(hwnd);
	}

	void NativeWindow::SetTitle(std::string_view _title) const
	{
		title = std::string(_title);
		if (hwnd != nullptr)
		{
			SetWindowTextW(hwnd, Utf8ToWide(_title).c_str());
		}
	}

	std::string_view NativeWindow::GetTitle() const
	{
		return title;
	}

	void NativeWindow::SetExtent(Extent2D<int> _extent)
	{
		if (hwnd == nullptr)
		{
			return;
		}

		const auto style = static_cast<DWORD>(GetWindowLongW(hwnd, GWL_STYLE));
		RECT rect = { 0, 0, _extent.x(), _extent.y() };
		AdjustWindowRect(&rect, style, FALSE);

		SetWindowPos(
			hwnd, nullptr, 0, 0,
			rect.right - rect.left,
			rect.bottom - rect.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	Extent2D<int> NativeWindow::GetExtent() const
	{
		RECT rect;
		if (hwnd != nullptr && GetClientRect(hwnd, &rect))
		{
			return Extent2D<int>{ static_cast<int>(rect.right - rect.left), static_cast<int>(rect.bottom - rect.top) };
		}

		return Extent2D<int>{ 0, 0 };
	}

	void NativeWindow::SetResizable(bool _resizable)
	{
		if (hwnd == nullptr)
		{
			return;
		}

		resizable = _resizable;
		LONG style = GetWindowLongW(hwnd, GWL_STYLE);

		if (_resizable)
		{
			style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}
		else
		{
			style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		}

		SetWindowLongW(hwnd, GWL_STYLE, style);
		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	bool NativeWindow::IsResizable() const
	{
		return resizable;
	}

	void NativeWindow::SetFullscreen(bool _fullscreen)
	{
		if (hwnd == nullptr || fullscreen == _fullscreen)
		{
			return;
		}

		fullscreen = _fullscreen;

		if (_fullscreen)
		{
			GetWindowPlacement(hwnd, &savedPlacement);

			const LONG style = GetWindowLongW(hwnd, GWL_STYLE);
			SetWindowLongW(hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

			MONITORINFO mi{ sizeof(MONITORINFO) };
			GetMonitorInfoW(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
			const RECT& r = mi.rcMonitor;

			SetWindowPos(hwnd, HWND_TOP,
				r.left, r.top, r.right - r.left, r.bottom - r.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
		else
		{
			const LONG style = GetWindowLongW(hwnd, GWL_STYLE);
			SetWindowLongW(hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
			SetWindowPlacement(hwnd, &savedPlacement);
			SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}

	bool NativeWindow::IsFullscreen() const
	{
		return fullscreen;
	}

	void NativeWindow::SetVSyncActive(bool _vsync)
	{
		vsyncActive = _vsync;
	}

	bool NativeWindow::IsVSyncActive() const
	{
		return vsyncActive;
	}

	void NativeWindow::FillRawInputState(RawInputState& _state) const
	{
		_state.heldKeys.assign(pressedKeys.begin(), pressedKeys.end());
		_state.mousePosition = mousePosition;
		_state.mouseButtons = mouseButtons;
		_state.mouseScroll = mouseScroll;

		mouseScroll = { 0.f, 0.f };
	}
}

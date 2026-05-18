#include "GLFWWindow.hpp"

#include <../../../Common/Public/Common/Core/Macros.hpp>

namespace
{
	cp::Key GlfwKeyToKey(const int _key)
	{
		if (_key >= GLFW_KEY_A && _key <= GLFW_KEY_Z)
			return static_cast<cp::Key>(static_cast<uint16_t>(cp::Key::A) + (_key - GLFW_KEY_A));
		if (_key >= GLFW_KEY_0 && _key <= GLFW_KEY_9)
			return static_cast<cp::Key>(static_cast<uint16_t>(cp::Key::D0) + (_key - GLFW_KEY_0));
		if (_key >= GLFW_KEY_F1 && _key <= GLFW_KEY_F12)
			return static_cast<cp::Key>(static_cast<uint16_t>(cp::Key::F1) + (_key - GLFW_KEY_F1));
		if (_key >= GLFW_KEY_KP_0 && _key <= GLFW_KEY_KP_9)
			return static_cast<cp::Key>(static_cast<uint16_t>(cp::Key::Numpad0) + (_key - GLFW_KEY_KP_0));

		switch (_key)
		{
		case GLFW_KEY_SPACE: return cp::Key::Space;
		case GLFW_KEY_ENTER: return cp::Key::Enter;
		case GLFW_KEY_ESCAPE: return cp::Key::Escape;
		case GLFW_KEY_TAB: return cp::Key::Tab;
		case GLFW_KEY_BACKSPACE: return cp::Key::Backspace;
		case GLFW_KEY_DELETE: return cp::Key::Delete;
		case GLFW_KEY_INSERT: return cp::Key::Insert;
		case GLFW_KEY_HOME: return cp::Key::Home;
		case GLFW_KEY_END: return cp::Key::End;
		case GLFW_KEY_PAGE_UP: return cp::Key::PageUp;
		case GLFW_KEY_PAGE_DOWN: return cp::Key::PageDown;
		case GLFW_KEY_LEFT: return cp::Key::Left;
		case GLFW_KEY_RIGHT: return cp::Key::Right;
		case GLFW_KEY_UP: return cp::Key::Up;
		case GLFW_KEY_DOWN: return cp::Key::Down;
		case GLFW_KEY_LEFT_SHIFT: return cp::Key::LeftShift;
		case GLFW_KEY_RIGHT_SHIFT: return cp::Key::RightShift;
		case GLFW_KEY_LEFT_CONTROL: return cp::Key::LeftCtrl;
		case GLFW_KEY_RIGHT_CONTROL: return cp::Key::RightCtrl;
		case GLFW_KEY_LEFT_ALT: return cp::Key::LeftAlt;
		case GLFW_KEY_RIGHT_ALT: return cp::Key::RightAlt;
		case GLFW_KEY_LEFT_SUPER: return cp::Key::LeftSuper;
		case GLFW_KEY_RIGHT_SUPER: return cp::Key::RightSuper;
		case GLFW_KEY_KP_ADD: return cp::Key::NumpadAdd;
		case GLFW_KEY_KP_SUBTRACT: return cp::Key::NumpadSubtract;
		case GLFW_KEY_KP_MULTIPLY: return cp::Key::NumpadMultiply;
		case GLFW_KEY_KP_DIVIDE: return cp::Key::NumpadDivide;
		case GLFW_KEY_KP_DECIMAL: return cp::Key::NumpadDecimal;
		case GLFW_KEY_KP_ENTER: return cp::Key::NumpadEnter;
		case GLFW_KEY_MINUS: return cp::Key::Minus;
		case GLFW_KEY_EQUAL: return cp::Key::Equal;
		case GLFW_KEY_LEFT_BRACKET: return cp::Key::LeftBracket;
		case GLFW_KEY_RIGHT_BRACKET: return cp::Key::RightBracket;
		case GLFW_KEY_BACKSLASH: return cp::Key::Backslash;
		case GLFW_KEY_SEMICOLON: return cp::Key::Semicolon;
		case GLFW_KEY_APOSTROPHE: return cp::Key::Apostrophe;
		case GLFW_KEY_GRAVE_ACCENT: return cp::Key::GraveAccent;
		case GLFW_KEY_COMMA: return cp::Key::Comma;
		case GLFW_KEY_PERIOD: return cp::Key::Period;
		case GLFW_KEY_SLASH: return cp::Key::Slash;
		case GLFW_KEY_CAPS_LOCK: return cp::Key::CapsLock;
		case GLFW_KEY_NUM_LOCK: return cp::Key::NumLock;
		case GLFW_KEY_SCROLL_LOCK: return cp::Key::ScrollLock;
		case GLFW_KEY_PRINT_SCREEN: return cp::Key::PrintScreen;
		case GLFW_KEY_PAUSE: return cp::Key::Pause;
		default: return cp::Key::Unknown;
		}
	}
}

#if defined(CP_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>

namespace cp
{
	GLFWWindow::GLFWWindow(const WindowInfo& _info) : IWindow(_info)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, _info.resizable ? GLFW_TRUE : GLFW_FALSE);

		window = glfwCreateWindow(_info.extent.x(), _info.extent.y(), _info.title.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(window, this);

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* _win, int _width, int _height)
		{
			if (_width > 0 && _height > 0)
			{
				auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(_win));
				if (self != nullptr)
					self->InvokeResizeCallback(Extent2D<int>{ _width, _height });
			}
		});

		glfwSetKeyCallback(window, [](GLFWwindow* _win, int _key, int, int _action, int)
		{
			const auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(_win));
			if (self == nullptr)
			{
				return;
			}

			const Key key = GlfwKeyToKey(_key);
			if (key == Key::Unknown)
			{
				return;
			}

			if (_action == GLFW_PRESS || _action == GLFW_REPEAT)
			{
				self->pressedKeys.insert(key);
			}
			else if (_action == GLFW_RELEASE)
			{
				self->pressedKeys.erase(key);
			}
		});

		glfwSetCursorPosCallback(window, [](GLFWwindow* _win, double _x, double _y)
		{
			const auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(_win));
			if (self != nullptr)
			{
				self->mousePosition = { static_cast<float>(_x), static_cast<float>(_y) };
			}
		});

		glfwSetScrollCallback(window, [](GLFWwindow* _win, double _xOffset, double _yOffset)
		{
			const auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(_win));
			if (self != nullptr)
			{
				self->mouseScroll += glm::vec2{ static_cast<float>(_xOffset), static_cast<float>(_yOffset) };
			}
		});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* _win, int _button, int _action, int /*_mods*/)
		{
			const auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(_win));
			if (self == nullptr || _button < 0 || _button >= static_cast<int>(MouseButton::Count))
			{
				return;
			}

			const uint32_t mask = 1u << static_cast<uint32_t>(_button);
			if (_action == GLFW_PRESS)
			{
				self->mouseButtons |= mask;
			}
			else if (_action == GLFW_RELEASE)
			{
				self->mouseButtons &= ~mask;
			}
		});

		glfwSetWindowFocusCallback(window, [](GLFWwindow* _win, int _focused)
		{
			if (!_focused)
			{
				const auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(_win));
				if (self != nullptr)
				{
					self->pressedKeys.clear();
					self->mouseButtons = 0;
				}
			}
		});
	}

	GLFWWindow::~GLFWWindow()
	{
		CleanUp();
	}

	void* GLFWWindow::GetNativeWindowHandle() const
	{
		return glfwGetWin32Window(window);
	}

	float GLFWWindow::GetAspectRatio() const
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		return static_cast<float>(width) / static_cast<float>(height);
	}

	void GLFWWindow::PollEvents() const
	{
		glfwPollEvents();
	}

	void GLFWWindow::WaitForEvents() const
	{
		glfwWaitEvents();
	}

	bool GLFWWindow::ShouldClose() const
	{
		return glfwWindowShouldClose(window);
	}

	void GLFWWindow::SetTitle(std::string_view _title) const
	{
		glfwSetWindowTitle(window, _title.data());
	}

	std::string_view GLFWWindow::GetTitle() const
	{
		return std::string_view(glfwGetWindowTitle(window));
	}

	void GLFWWindow::SetExtent(Extent2D<int> _extent)
	{
		glfwSetWindowSize(window, _extent.x(), _extent.y());
	}

	Extent2D<int> GLFWWindow::GetExtent() const
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		
		return Extent2D<int>{ width, height };
	}

	void GLFWWindow::SetResizable(bool _resizable)
	{
		glfwSetWindowAttrib(window, GLFW_RESIZABLE, _resizable ? GLFW_TRUE : GLFW_FALSE);
	}

	bool GLFWWindow::IsResizable() const
	{
		return glfwGetWindowAttrib(window, GLFW_RESIZABLE) == GLFW_TRUE;
	}

	void GLFWWindow::SetFullscreen(bool _fullscreen)
	{
		glfwSetWindowMonitor(window, _fullscreen ? glfwGetPrimaryMonitor() : nullptr, 0, 0, 0, 0, GLFW_DONT_CARE);
	}

	bool GLFWWindow::IsFullscreen() const
	{
		return glfwGetWindowMonitor(window) != nullptr;
	}

	void GLFWWindow::SetVSyncActive(bool _vsync)
	{
		
	}

	bool GLFWWindow::IsVSyncActive() const
	{
		return false;
	}

	void GLFWWindow::CleanUp() const
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void GLFWWindow::FillRawInputState(RawInputState& _state) const
	{
		_state.heldKeys.assign(pressedKeys.begin(), pressedKeys.end());
		_state.mousePosition = mousePosition;
		_state.mouseButtons = mouseButtons;
		_state.mouseScroll = mouseScroll;

		mouseScroll = { 0.f, 0.f };
	}
}
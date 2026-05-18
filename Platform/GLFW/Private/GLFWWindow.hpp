#pragma once

#include <IWindow.hpp>
#include <GLFW/glfw3.h>

#include <../../../Common/Public/Common/Data/Extent.hpp>

#include <cstdint>
#include <unordered_set>

namespace cp
{
	class GLFWWindow : public IWindow
	{
	private:
		GLFWwindow* window = nullptr;

		mutable std::unordered_set<Key> pressedKeys;
		mutable glm::vec2 mousePosition { 0.f, 0.f };
		mutable glm::vec2 mouseScroll { 0.f, 0.f };
		mutable uint32_t mouseButtons = 0;

	public:
		GLFWWindow(const WindowInfo& _info);
		~GLFWWindow();

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
	};
}
#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtTheme final : public cp::editorui::ITheme
	{
	public:
		explicit QtTheme(std::string _name)
			: name(std::move(_name))
		{
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::WindowBackground)] = { 36, 37, 43, 255 };
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::PanelBackground)] = { 43, 45, 53, 255 };
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::PanelBorder)] = { 62, 65, 75, 255 };
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::TextPrimary)] = { 220, 223, 228, 255 };
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::TextSecondary)] = { 163, 168, 176, 255 };
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::Accent)] = { 82, 156, 255, 255 };
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::Selection)] = { 65, 102, 159, 255 };
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::Warning)] = { 234, 179, 8, 255 };
			colors[static_cast<size_t>(cp::editorui::ThemeColorRole::Error)] = { 239, 68, 68, 255 };
		}

		[[nodiscard]] std::string_view GetName() const override
		{
			return name;
		}

		void SetColor(const cp::editorui::ThemeColorRole _role, const cp::editorui::RGBA8 _color) override
		{
			colors[static_cast<size_t>(_role)] = _color;
		}

		[[nodiscard]] cp::editorui::RGBA8 GetColor(const cp::editorui::ThemeColorRole _role) const override
		{
			return colors[static_cast<size_t>(_role)];
		}

	private:
		std::string name;
		std::array<cp::editorui::RGBA8, 9> colors{};
	};
}


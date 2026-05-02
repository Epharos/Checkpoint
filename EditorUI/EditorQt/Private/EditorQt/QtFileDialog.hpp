#pragma once

#include <EditorUI/Widgets.hpp>
#include <filesystem>
#include <optional>
#include <vector>

class QFileDialog;

namespace cp::editorqt
{
	class QtFileDialog final : public cp::editorui::IFileDialog
	{
	public:
		explicit QtFileDialog(std::string _id);
		~QtFileDialog() override;

		std::string_view GetId() const override;
		void SetVisible(bool _visible) override;
		bool IsVisible() const override;
		void SetEnabled(bool _enabled) override;
		bool IsEnabled() const override;

		void SetMode(Mode _mode) override;
		Mode GetMode() const override;
		void SetTitle(std::string _title) override;
		void SetDirectory(std::filesystem::path _directory) override;
		void SetFilter(std::string _filter) override;
		void SetDefaultFileName(std::string _fileName) override;
		std::optional<std::filesystem::path> GetSelectedPath() const override;
		std::vector<std::filesystem::path> GetSelectedPaths() const override;
		bool Execute() override;

	private:
		std::string id;
		Mode mode = Mode::OpenFile;
		std::filesystem::path directory;
		std::string filter;
		std::string defaultFileName;
		std::vector<std::filesystem::path> selectedPaths;
		bool visible = false;
		bool enabled = true;
	};
}

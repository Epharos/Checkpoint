#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <EditorUI/Widgets.hpp>

namespace cp::editorui
{
	class IFileDrop : public IWidget
	{
	public:
		using FileDropCallback = std::function<void(const std::filesystem::path&)>;

		virtual ~IFileDrop() = default;

		virtual void SetFileExtensions(const std::vector<std::string>& _extensions) = 0;
		virtual const std::vector<std::string>& GetFileExtensions() const = 0;

		virtual void SetFileDropCallback(FileDropCallback _callback) = 0;

		virtual void SetFilePath(const std::filesystem::path& _path) = 0;
		[[nodiscard]] virtual std::filesystem::path GetFilePath() const = 0;

		virtual void SetPlaceholderText(std::string _text) = 0;
		virtual std::string GetPlaceholderText() const = 0;

		virtual void Clear() = 0;
	};
}

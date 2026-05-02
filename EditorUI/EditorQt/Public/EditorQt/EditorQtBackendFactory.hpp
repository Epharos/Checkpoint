#pragma once

#include <memory>

#include <EditorUI/Backend.hpp>

namespace cp::editorqt
{
	std::shared_ptr<cp::editorui::IEditorUIBackend> CreateEditorQtBackend();
}

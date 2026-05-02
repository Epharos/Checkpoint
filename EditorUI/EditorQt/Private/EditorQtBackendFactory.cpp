#include "../Public/EditorQt/EditorQtBackendFactory.hpp"

#include "EditorQt/QtEditorUIBackend.hpp"

namespace cp::editorqt
{
	std::shared_ptr<cp::editorui::IEditorUIBackend> CreateEditorQtBackend()
	{
		return std::make_shared<QtEditorUIBackend>();
	}
}

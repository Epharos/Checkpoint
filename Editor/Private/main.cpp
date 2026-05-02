#include <Editor/EditorWorkspace.hpp>

#include <EditorQt/EditorQtBackendFactory.hpp>

#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
	std::puts("Plop");

	try
	{
		const std::shared_ptr<cp::editorui::IEditorUIBackend> backend = cp::editorqt::CreateEditorQtBackend();

		cp::editor::EditorWorkspace workspace(backend);

		cp::editor::WorkspaceConfig config;
		config.windowTitle = "Checkpoint Editor";
		if (argc > 1 && argv[1] != nullptr)
		{
			config.projectRootPath = std::filesystem::absolute(argv[1]);
		}
		else
		{
			config.projectRootPath = std::filesystem::current_path();
		}
		if (argc > 0 && argv[0] != nullptr)
		{
			config.executablePath = std::filesystem::absolute(argv[0]);
		}

		workspace.BuildDefaultLayout(config);
		return 0;
	}
	catch (const std::exception& exception)
	{
		std::cerr << "Editor startup failed: " << exception.what() << '\n';
		return 1;
	}
}

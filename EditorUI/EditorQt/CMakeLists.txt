add_library(EditorUI_EditorQt STATIC
    "Private/EditorQtBackendFactory.cpp"
    "Private/EditorQt/EditorQtCommon.hpp"
    "Private/EditorQt/QtAction.hpp"
    "Private/EditorQt/QtContextMenu.hpp"
    "Private/EditorQt/QtFileDialog.hpp"
    "Private/EditorQt/QtFileDialog.cpp"
    "Private/EditorQt/QtSearchBox.hpp"
    "Private/EditorQt/QtSceneHierarchyView.hpp"
    "Private/EditorQt/QtAssetExplorerView.hpp"
    "Private/EditorQt/QtConsoleView.hpp"
    "Private/EditorQt/QtFileDropWidget.hpp"
    "Private/EditorQt/QtInspectorView.hpp"
    "Private/EditorQt/QtViewportWidget.hpp"
    "Private/EditorQt/QtToolBar.hpp"
    "Private/EditorQt/QtMenu.hpp"
    "Private/EditorQt/QtMenuBar.hpp"
    "Private/EditorQt/QtDockPanel.hpp"
    "Private/EditorQt/QtDockHost.hpp"
    "Private/EditorQt/QtTheme.hpp"
    "Private/EditorQt/QtCommandStack.hpp"
    "Private/EditorQt/QtActionRegistry.hpp"
    "Private/EditorQt/QtApplicationWindow.hpp"
    "Private/EditorQt/QtEditorUIBackend.hpp"
    "Public/EditorQt/EditorQtBackendFactory.hpp"
    "Private/EditorQtPCH.hpp"
)

target_include_directories(EditorUI_EditorQt
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_precompile_headers(EditorUI_EditorQt PRIVATE
    "Private/EditorQtPCH.hpp"
)

target_link_libraries(EditorUI_EditorQt
    PUBLIC
        EditorUI
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
)

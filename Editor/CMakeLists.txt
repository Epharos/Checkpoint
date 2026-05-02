add_executable(Editor
    "Private/EditorWorkspace.cpp"
    "Private/main.cpp"
    "Public/Editor/EditorWorkspace.hpp"
    "Private/EditorPCH.hpp"
)

target_include_directories(Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_precompile_headers(Editor PRIVATE
    "Private/EditorPCH.hpp"
)

target_link_libraries(Editor
    PUBLIC
        Common
        ECS
        Rendering
        Resources
        RHI_Core
        RHI_Vulkan
        EditorUI
        EditorUI_EditorQt
)

add_library(EditorUI INTERFACE)

target_include_directories(EditorUI
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_compile_features(EditorUI
    INTERFACE
        cxx_std_20
)

add_subdirectory(EditorQt)

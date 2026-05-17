add_library(Runtime_Editor STATIC
    "Private/RuntimeEditorApplication.cpp"
    "Public/RuntimeEditorApplication.hpp"
)

target_include_directories(Runtime_Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Runtime_Editor
    PUBLIC
        Common
        ECS
        Runtime
        Rendering
        RHI_Core
        Platform_Native
)

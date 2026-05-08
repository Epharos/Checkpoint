add_library(Runtime STATIC
    "Private/Scene/Scene.cpp"
    "Private/Scene/SceneSerializer.cpp"
    "Private/Scene/SceneManager.cpp"
    "Private/Scene/FrameGraphSerializer.cpp"

    "Public/Runtime/Scene/Scene.hpp"
    "Public/Runtime/Scene/SceneSerializer.hpp"
    "Public/Runtime/Scene/SceneManager.hpp"
    "Public/Runtime/Scene/FrameGraphSerializer.hpp"
)

target_include_directories(Runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
        ${CMAKE_CURRENT_SOURCE_DIR}/Standalone/Public
)

target_link_libraries(Runtime
    PUBLIC
        Common
        ECS
        Rendering
)

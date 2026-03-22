add_library(Rendering STATIC
        Private/Renderer.cpp
        Private/Renderer.hpp
        Public/Rendering/Renderer.hpp)

target_include_directories(Rendering
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Rendering
    PUBLIC
        Common
        RHI_Core
        Resources
)
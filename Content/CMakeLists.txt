file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

add_library(ExampleContent STATIC ${CORE_SOURCES})

target_include_directories(ExampleContent
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${STB_INCLUDE_DIR}             # stb header
)

# target_precompile_headers(Core
#     PRIVATE
#         src/pch.hpp
# )

target_link_libraries(ExampleContent
    PUBLIC
        Vulkan::Vulkan
        glm
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
)

target_compile_definitions(ExampleContent
    PUBLIC
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)
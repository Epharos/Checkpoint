file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

add_library(EngineWidgets ${EDITOR_SOURCES})

target_include_directories(EngineWidgets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(EngineWidgets
    PUBLIC
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
)

target_compile_definitions(EngineWidgets
    PUBLIC
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)
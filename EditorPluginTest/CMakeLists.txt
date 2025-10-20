file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

add_library(EditorPluginTest SHARED ${CORE_SOURCES})

target_include_directories(EditorPluginTest
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(EditorPluginTest PRIVATE
  FILE_SET cxx_modules
    TYPE CXX_MODULES
    FILES 
    "Main.ixx" )

target_link_libraries(EditorPluginTest
    PUBLIC
        EditorAPI
)

target_compile_definitions(EditorPluginTest
    PUBLIC
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)
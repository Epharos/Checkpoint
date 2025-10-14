file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

add_library(Core STATIC ${CORE_SOURCES})

target_include_directories(Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${STB_INCLUDE_DIR}             # stb header
)

target_precompile_headers(Core
    PRIVATE
        src/pch.hpp
)

target_link_libraries(Core
    PUBLIC
        Vulkan::Vulkan
        glm
        glfw
        SPIRV-Tools-opt                # main spirv-tools target
        slang
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
        assimp
        nlohmann_json
        EngineWidgets                 #Temporary
)

target_compile_definitions(Core
    PUBLIC
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)
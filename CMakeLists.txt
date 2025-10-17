cmake_minimum_required (VERSION 3.31)

set(CMAKE_AUTOMOC ON)

add_compile_options(/Zc:__cplusplus)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(Checkpoint LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(EDITOR_BUILD "Editor Build" ON)
option(EXAMPLE_BUILD "Example Build" OFF)

include(FetchContent)

set(STB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/stb)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")

find_package(Vulkan REQUIRED)
find_package(Qt6 6.9 COMPONENTS Widgets Gui Core REQUIRED)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    slang
    GIT_REPOSITORY https://github.com/shader-slang/slang.git
    GIT_TAG v2025.18.1
)

set(SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC OFF) #12 hours ...

FetchContent_MakeAvailable(slang)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.2
)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

#add_dependencies(Core EngineWidgets) #Temporary

add_subdirectory(Core)
add_subdirectory(EngineWidgets) #Temporary

if(EDITOR_BUILD)
    add_subdirectory(Editor)
    add_compile_definitions(IN_EDITOR) #Adds an "IN_EDITOR" pre-processor when building Core for Editor
endif()

if(EXAMPLE_BUILD)
    add_subdirectory(Example)
    add_compile_definitions(IS_EXAMPLE_SAMPLE) #Adds an "IS_EXAMPLE_SAMPLE" pre-processor when building Core for Example
endif()

function(copy_to_build TARGET)
    foreach(PATH IN LISTS ARGN)
        if(EXISTS "${PATH}")
            get_filename_component(NAME ${PATH} NAME)
            if(IS_DIRECTORY "${PATH}")
                add_custom_command(TARGET ${TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                            "${PATH}"
                            "$<TARGET_FILE_DIR:${TARGET}>/${NAME}"
                    COMMENT "Copying directory ${NAME}..."
                )
            else()
                add_custom_command(TARGET ${TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${PATH}"
                            "$<TARGET_FILE_DIR:${TARGET}>/${NAME}"
                    COMMENT "Copying file ${NAME}..."
                )
            endif()
        else()
            message(WARNING "Path not found: ${PATH}")
        endif()
    endforeach()
endfunction()
cmake_minimum_required (VERSION 3.31...4.2.3)

###################
# Global Settings #
###################

set(CMAKE_AUTOMOC ON)

# add_compile_options(/Zc:__cplusplus)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(
        Checkpoint
        VERSION 1.0
        LANGUAGES CXX
) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

#########################
# External Dependencies #
#########################

include(FetchContent)

set(STB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/stb)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")

set(SLANG_ENABLE_DXC OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_USE_PREBUILT OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED)
find_package(Qt6 6.9 COMPONENTS Widgets Gui Core Svg REQUIRED)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    slang
    GIT_REPOSITORY https://github.com/shader-slang/slang.git
    GIT_TAG v2025.18.1
    EXCLUDE_FROM_ALL
)

set(SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC OFF) #12 hours ...

FetchContent_MakeAvailable(slang)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.2
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(nlohmann_json)

###################
# Project subdirs #
###################

add_subdirectory(Common) # Common structures and methods : Math, Allocators, Data Structures, Etc.
add_subdirectory(RHI) # Rendering Hardware Interface : Vulkan and DirectX12 backends
add_subdirectory(RHI/Vulkan) # Vulkan RHI backend
add_subdirectory(RHI/DirectX12) # DirectX12 RHI backend
add_subdirectory(Platform) # Platform abstraction layer : Windowing, Input, Etc.
add_subdirectory(Platform/GLFW) # GLFW implementation of the platform layer
add_subdirectory(Resources) # Asset management : Loading, Caching, Etc.
add_subdirectory(Rendering)  # Rendering module : Framegraph, Materials, Etc.
add_subdirectory(ECS) # Entity Component System
add_subdirectory(Builder) # Executable Builder module : Turning project files into a runnable executable
add_subdirectory(Runtime) # Common Runtime structures (Scene)
add_subdirectory(Runtime/Standalone) # Standalone Runtime module : Game loop, Etc.
add_subdirectory(Runtime/Editor) # Editor Runtime module : in-editor play mode
add_subdirectory(Launcher) # Editor launcher module : Starts the editor with a selected project
add_subdirectory(EditorUI) # Editor UI abstraction layer and backends
add_subdirectory(Editor) # Editor module : orchestrates editor tools and UI composition
add_subdirectory(ExamplePlugin) # Example Plugin : used to test features

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

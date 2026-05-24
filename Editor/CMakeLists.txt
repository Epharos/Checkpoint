add_executable(Editor
    "Private/EditorWorkspace.cpp"
    "Private/main.cpp"
    "Public/Editor/EditorWorkspace.hpp"
    "Private/KeybindRegistry.hpp"
    "Private/KeybindRegistry.cpp"
    "Private/EditorPCH.hpp"
    "Private/ShaderCacheProvider.hpp"
    "Private/UserPluginManager.hpp"
    "Private/UserPluginManager.cpp"
)

target_include_directories(Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_precompile_headers(Editor PRIVATE
    "Private/EditorPCH.hpp"
)

target_compile_definitions(Editor PRIVATE
    CP_ENGINE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    CP_CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
    CP_CMAKE_C_COMPILER="${CMAKE_C_COMPILER}"
)

target_link_libraries(Editor
    PUBLIC
        Common
        ECS
        Rendering
        Runtime
        Runtime_Editor
        Resources
        RHI_Core
        RHI_Vulkan
        EditorUI
        EditorUI_EditorQt
        Builder
        ShaderCompiler
        nlohmann_json::nlohmann_json
)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:slang>"
        "$<TARGET_FILE_DIR:Editor>/slang.dll"
    COMMENT "Copying slang.dll to Editor output"
    VERBATIM
)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/Editor_Resources"
        "$<TARGET_FILE_DIR:Editor>/Editor_Resources"
    COMMENT "Copying Editor_Resources to output directory"
    VERBATIM
)

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET Editor POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-compiler-runtime
                "$<TARGET_FILE:Editor>"
            COMMENT "Deploying Qt DLLs"
            VERBATIM
        )
    endif()
endif()

# ---------------------------------------------------------------------------
# SDK export — runs after every Editor build.
# Mirrors each engine library's Public/ and Private/ directories under
# sdk/<LibName>/ so that ALL relative #include paths resolve identically
# to the source tree — whether the include goes from public→private or
# from private→public.
#
# Output layout:
#   sdk/lib/<LibName>.lib              — import lib (renamed to canonical form)
#   sdk/<LibName>/Public/  (→ added to plugin include path)
#   sdk/<LibName>/Private/ (→ resolves relative includes transparently)
#
# Example: Common/Public/Common/Core/Assert.hpp does
#     #include "../../../Private/Core/Assert.hpp"
#   In the SDK that resolves to sdk/Common/Private/Core/Assert.hpp ✓
#   And if that private header does #include "../../Public/.." it resolves
#   back to sdk/Common/Public/... ✓
# ---------------------------------------------------------------------------
set(_cp_sdk_libs
    Common
    ECS
    Rendering
    Resources
    RHI_Core
    Runtime
    Platform
    ShaderCompiler
)

foreach(_lib ${_cp_sdk_libs})
    # -- import lib (.lib) --
    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:Editor>/sdk/lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_LINKER_FILE:${_lib}>"
            "$<TARGET_FILE_DIR:Editor>/sdk/lib/${_lib}.lib"
        COMMENT "SDK: exporting ${_lib} import lib"
        VERBATIM
    )

    # -- public headers → sdk/<LibName>/Public/ --
    if(EXISTS "${CMAKE_SOURCE_DIR}/${_lib}/Public")
        add_custom_command(TARGET Editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:Editor>/sdk/${_lib}/Public"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/${_lib}/Public"
                "$<TARGET_FILE_DIR:Editor>/sdk/${_lib}/Public"
            COMMENT "SDK: exporting ${_lib} public headers"
            VERBATIM
        )
    endif()

    # -- private headers → sdk/<LibName>/Private/ --
    # Not added to the include path; relative includes from public headers
    # reach them automatically since the directory tree mirrors the source.
    if(EXISTS "${CMAKE_SOURCE_DIR}/${_lib}/Private")
        add_custom_command(TARGET Editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:Editor>/sdk/${_lib}/Private"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/${_lib}/Private"
                "$<TARGET_FILE_DIR:Editor>/sdk/${_lib}/Private"
            COMMENT "SDK: exporting ${_lib} private headers"
            VERBATIM
        )
    endif()
endforeach()

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Editor>/sdk/RHI_Core/Public"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/RHI/Public"
        "$<TARGET_FILE_DIR:Editor>/sdk/RHI_Core/Public"
    COMMENT "SDK: exporting RHI_Core public headers"
    VERBATIM
)

if(EXISTS "${CMAKE_SOURCE_DIR}/RHI/Private")
    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:Editor>/sdk/RHI_Core/Private"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/RHI/Private"
            "$<TARGET_FILE_DIR:Editor>/sdk/RHI_Core/Private"
        COMMENT "SDK: exporting RHI_Core private headers"
        VERBATIM
    )
endif()

# ---------------------------------------------------------------------------
# Third-party transitive deps — plugins that link Resources or ShaderCompiler
# need these at link time because those modules are static libraries.
# ---------------------------------------------------------------------------
add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_LINKER_FILE:assimp::assimp>"
        "$<TARGET_FILE_DIR:Editor>/sdk/lib/assimp.lib"
    COMMENT "SDK: exporting assimp lib"
    VERBATIM
)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_LINKER_FILE:zlibstatic>"
        "$<TARGET_FILE_DIR:Editor>/sdk/lib/zlib.lib"
    COMMENT "SDK: exporting zlib lib (assimp dep)"
    VERBATIM
)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_LINKER_FILE:slang>"
        "$<TARGET_FILE_DIR:Editor>/sdk/lib/slang.lib"
    COMMENT "SDK: exporting slang lib"
    VERBATIM
)

# ---------------------------------------------------------------------------
# EngineSDK.cmake — consumed by the plugin's generated CMakeLists.
# Defines cp::<LibName> IMPORTED targets with their INTERFACE_INCLUDE_DIRECTORIES
# and INTERFACE_LINK_LIBRARIES so that CMake propagates transitive deps
# automatically when the plugin calls target_link_libraries(... PRIVATE cp::Foo).
# Generated at CMake configure time (file(GENERATE) evaluates generator
# expressions); parent directory is created automatically.
# ---------------------------------------------------------------------------
file(GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/sdk/EngineSDK.cmake"
    CONTENT [[
# Auto-generated by the engine build — do not edit.
cmake_minimum_required(VERSION 3.20)

get_filename_component(_cp_sdk_dir "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
set(_cp_sdk_lib_dir "${_cp_sdk_dir}/lib")

# ── Engine static libraries ────────────────────────────────────────────────
foreach(_cp_lib Common ECS Rendering Resources RHI_Core Runtime Platform ShaderCompiler)
    if(NOT TARGET cp::${_cp_lib})
        add_library(cp::${_cp_lib} STATIC IMPORTED)
        set_target_properties(cp::${_cp_lib} PROPERTIES
            IMPORTED_LOCATION             "${_cp_sdk_lib_dir}/${_cp_lib}.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${_cp_sdk_dir}/${_cp_lib}/Public"
        )
    endif()
endforeach()

# ── Third-party transitive deps ────────────────────────────────────────────
# zlib  (pulled in by assimp)
if(NOT TARGET cp::_zlib)
    add_library(cp::_zlib STATIC IMPORTED)
    set_target_properties(cp::_zlib PROPERTIES
        IMPORTED_LOCATION "${_cp_sdk_lib_dir}/zlib.lib"
    )
endif()

# assimp  (needed by cp::Resources)
if(NOT TARGET cp::_assimp)
    add_library(cp::_assimp STATIC IMPORTED)
    set_target_properties(cp::_assimp PROPERTIES
        IMPORTED_LOCATION        "${_cp_sdk_lib_dir}/assimp.lib"
        INTERFACE_LINK_LIBRARIES "cp::_zlib"
    )
    set_target_properties(cp::Resources PROPERTIES
        INTERFACE_LINK_LIBRARIES "cp::_assimp"
    )
endif()

# slang  (needed by cp::ShaderCompiler)
if(NOT TARGET cp::_slang)
    add_library(cp::_slang UNKNOWN IMPORTED)
    set_target_properties(cp::_slang PROPERTIES
        IMPORTED_LOCATION "${_cp_sdk_lib_dir}/slang.lib"
    )
    set_target_properties(cp::ShaderCompiler PROPERTIES
        INTERFACE_LINK_LIBRARIES "cp::_slang"
    )
endif()
]]
)

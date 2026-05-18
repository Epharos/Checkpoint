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

file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

add_executable(Editor ${EDITOR_SOURCES})

# ---- Folders ----
set(DIRS_TO_COPY
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/DEBUG"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/Editor_Resources"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/platforms"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/PROJECTS"
)

foreach(DIR ${DIRS_TO_COPY})
    if(EXISTS "${DIR}")
        get_filename_component(DIR_NAME ${DIR} NAME)
        add_custom_command(TARGET Editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${DIR}"
                    "$<TARGET_FILE_DIR:Editor>/${DIR_NAME}"
            COMMENT "Copying directory ${DIR_NAME} to Editor output..."
        )
    else()
        message(WARNING "Directory not found: ${DIR}")
    endif()
endforeach()

# ---- Files ----
set(FILES_TO_COPY
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/Qt6Core.dll"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/Qt6Cored.dll"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/Qt6Gui.dll"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/Qt6Guid.dll"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/Qt6Widgets.dll"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/Qt6Widgetsd.dll"
    "${CMAKE_SOURCE_DIR}/TO_INCLUDE_IN_BUILD/recent_projects.json"
)

foreach(FILE ${FILES_TO_COPY})
    if(EXISTS "${FILE}")
        get_filename_component(FILE_NAME ${FILE} NAME)
        add_custom_command(TARGET Editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${FILE}"
                    "$<TARGET_FILE_DIR:Editor>/${FILE_NAME}"
            COMMENT "Copying file ${FILE_NAME} to Editor output..."
        )
    else()
        message(WARNING "File not found: ${FILE}")
    endif()
endforeach()

target_include_directories(Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${STB_INCLUDE_DIR}             # stb header
        ${CMAKE_SOURCE_DIR}/Core/Include
        ${CMAKE_SOURCE_DIR}/EngineWidgets/Include #Temporary
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Editor
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
        Core
        EngineWidgets                 #Temporary
)

target_compile_definitions(Editor
    PUBLIC
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)
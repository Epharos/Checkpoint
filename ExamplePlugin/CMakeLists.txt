add_library(ExamplePlugin SHARED
    "Private/ExamplePlugin.cpp"
    "Private/ExamplePluginRuntime.cpp"
    "Private/ExamplePluginEditor.cpp"
    "Private/Components/Components.hpp"
    "Private/Components/ComponentRegistrationContext.hpp"
    "Private/Components/Transform.hpp"
    "Private/Components/Transform.cpp"
    "Private/Components/Camera.hpp"
    "Private/Components/Camera.cpp"
    "Private/Components/MeshRenderer.hpp"
    "Private/Components/MeshRenderer.cpp"
    "Private/Components/Spawner.hpp"
    "Private/Components/Spawner.cpp"
    "Private/Components/RigidBody.hpp"
    "Private/Components/RigidBody.cpp"
    "Private/RenderPasses/OpaqueMaterialPass.hpp"
    "Private/ExamplePluginPCH.hpp"
)

target_include_directories(ExamplePlugin
    PUBLIC
        ${CMAKE_SOURCE_DIR}/Common/Public
)

target_precompile_headers(ExamplePlugin PRIVATE
    "Private/ExamplePluginPCH.hpp"
)

target_link_libraries(ExamplePlugin
    PUBLIC
        Common
        ECS
        Rendering
        RHI_Core
    PRIVATE
        glm::glm
)

if(TARGET Runtime_Standalone)
    add_dependencies(Runtime_Standalone ExamplePlugin)

    add_custom_command(TARGET ExamplePlugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Runtime_Standalone>/Plugins"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:ExamplePlugin>" "$<TARGET_FILE_DIR:Runtime_Standalone>/Plugins/$<TARGET_FILE_NAME:ExamplePlugin>"
        COMMENT "Copying ExamplePlugin to Runtime_Standalone/Plugins"
    )
endif()

if(TARGET Editor)
    add_dependencies(Editor ExamplePlugin)

    add_custom_command(TARGET ExamplePlugin POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Editor>/Plugins"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:ExamplePlugin>" "$<TARGET_FILE_DIR:Editor>/Plugins/$<TARGET_FILE_NAME:ExamplePlugin>"
            COMMENT "Copying ExamplePlugin to Editor/Plugins"
    )
endif()

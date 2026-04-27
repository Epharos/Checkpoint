add_library(ExamplePlugin SHARED
    "Private/ExamplePlugin.cpp"
    "Private/Components/RenderingComponents.hpp"
    "Private/Components/RenderingComponentRegistrars.hpp"
    "Private/RenderPasses/OpaqueMaterialPass.hpp"
)

target_include_directories(ExamplePlugin
    PUBLIC
        ${CMAKE_SOURCE_DIR}/Common/Public
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

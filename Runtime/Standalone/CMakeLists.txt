add_executable(Runtime_Standalone
    "Private/StandaloneApplication.cpp"
    "Private/StandaloneRuntime.cpp"
)

target_include_directories(Runtime_Standalone
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Runtime_Standalone
    PUBLIC
        Common
        ECS
        Platform_GLFW # By the end it should be just Platform and load the corresponding platform dynamically
        RHI_Vulkan # By the end it should be just RHI and load the corresponding RHI dynamically
        Rendering
        Resources
        Runtime
)

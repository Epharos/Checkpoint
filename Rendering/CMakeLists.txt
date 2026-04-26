add_library(Rendering STATIC
        Private/Renderer.cpp
        Private/Renderer.hpp

        Private/FrameGraph/Renderpass.cpp
        Private/FrameGraph/Renderpass.hpp
        Private/FrameGraph/FramegraphResource.cpp
        Private/FrameGraph/FramegraphResource.hpp
        Private/FrameGraph/FrameGraphBuilder.cpp
        Private/FrameGraph/FrameGraphBuilder.hpp
        Private/FrameGraph/FrameGraph.cpp
        Private/FrameGraph/FrameGraph.hpp
        Private/FrameGraph/BarrierManager.cpp
        Private/FrameGraph/BarrierManager.hpp
        Private/FrameGraph/PassDependencyManager.cpp
        Private/FrameGraph/PassDependencyManager.hpp

        Public/Rendering/Renderer.hpp
)

target_include_directories(Rendering
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Rendering
    PUBLIC
        Common
        RHI_Core
        Resources
        ECS
)

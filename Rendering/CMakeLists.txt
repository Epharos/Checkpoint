add_library(Rendering STATIC
        Private/Renderer.cpp
        Private/Renderer.hpp
        Private/Camera.hpp
        Private/DebugDraw.hpp
        Private/RendererEnvironment.hpp

        Private/BuiltinRenderPasses.cpp
        Private/RenderPasses/DebugDrawPass.hpp
        Private/RenderPasses/EditorGridPass.hpp
        Private/RenderPasses/EditorGridPassAuthoring.hpp
        Private/Shaders/EditorGrid.slang
        Private/Shaders/DebugLine.slang

        Public/Rendering/IShaderProvider.hpp

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
        Public/Rendering/Camera.hpp
        Public/Rendering/BuiltinRenderPasses.hpp
        Public/Rendering/DebugDraw.hpp
        Public/Rendering/GizmoContribution.hpp
        Public/Rendering/RendererEnvironment.hpp

        Public/Rendering/Scene/FrameGraphConfig.hpp
        Private/Scene/FrameGraphConfig.cpp
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
        glm::glm
)

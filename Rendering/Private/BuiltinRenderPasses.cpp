#include <Rendering/BuiltinRenderPasses.hpp>

#include <Common/Core/Registry.hpp>
#include <Common/Plugin/RenderPassAuthoring.hpp>

#include "RenderPasses/DebugDrawPass.hpp"
#include "RenderPasses/EditorGridPass.hpp"
#include "RenderPasses/EditorGridPassAuthoring.hpp"

namespace cp::rendering
{
    void RegisterBuiltinRenderPasses(Registry<IRenderPass>& _registry)
    {
        _registry.RegisterType<DebugDrawPass>(std::string(DebugDrawPassTypeName));
        _registry.RegisterType<EditorGridPass>(std::string(EditorGridPassTypeName));
    }

    void UnregisterBuiltinRenderPasses(Registry<IRenderPass>& _registry)
    {
        _registry.Unregister(DebugDrawPassTypeName);
        _registry.Unregister(EditorGridPassTypeName);
    }

    void RegisterBuiltinRenderPassesAuthoring(Registry<IRenderPassAuthoring>& _registry)
    {
        _registry.RegisterType<EditorGridPassAuthoring>(std::string(EditorGridPassTypeName));
    }

    void UnregisterBuiltinRenderPassesAuthoring(Registry<IRenderPassAuthoring>& _registry)
    {
        _registry.Unregister(EditorGridPassTypeName);
    }
}

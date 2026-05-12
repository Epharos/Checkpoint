#include <Rendering/BuiltinRenderPasses.hpp>

#include <Common/Core/Registry.hpp>

#include "RenderPasses/DebugDrawPass.hpp"

namespace cp::rendering
{
    void RegisterBuiltinRenderPasses(Registry<IRenderPass>& _registry)
    {
        _registry.RegisterType<DebugDrawPass>(std::string(DebugDrawPassTypeName));
    }

    void UnregisterBuiltinRenderPasses(Registry<IRenderPass>& _registry)
    {
        _registry.Unregister(DebugDrawPassTypeName);
    }
}

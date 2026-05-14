#pragma once

namespace cp
{
    template<typename T> class Registry;
    class IRenderPass;
    class IRenderPassAuthoring;

    namespace rendering
    {
        void RegisterBuiltinRenderPasses(Registry<IRenderPass>& _registry);
        void UnregisterBuiltinRenderPasses(Registry<IRenderPass>& _registry);

        void RegisterBuiltinRenderPassesAuthoring(Registry<IRenderPassAuthoring>& _registry);
        void UnregisterBuiltinRenderPassesAuthoring(Registry<IRenderPassAuthoring>& _registry);
    }
}

#pragma once

namespace cp
{
    template<typename T> class Registry;
    class IRenderPass;

    namespace rendering
    {
        void RegisterBuiltinRenderPasses(Registry<IRenderPass>& _registry);
        void UnregisterBuiltinRenderPasses(Registry<IRenderPass>& _registry);
    }
}

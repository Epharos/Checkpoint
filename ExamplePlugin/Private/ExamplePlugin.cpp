#include <Common/Core/Log.hpp>
#include <Common/Plugin/PluginAPI.hpp>

#include "RenderPasses/SceneRenderPass.hpp"
#include "RenderPasses/NegativePFX.hpp"

namespace
{
	inline constexpr std::string_view RenderPassRegistryName = "Renderpass";
	inline constexpr std::string_view SceneRenderPassTypeName = "SceneRenderPass";
	inline constexpr std::string_view NegativePostFXTypeName = "NegativePostFX";

	bool RegisterExamplePlugin(cp::PluginHostContext& _context)
	{
		if (_context.registryManager == nullptr)
		{
			return false;
		}

		cp::Registry<cp::IRenderPass>& renderPassRegistry =
			_context.registryManager->GetOrCreate<cp::IRenderPass>(std::string(RenderPassRegistryName));

		const bool sceneRegistered =
			renderPassRegistry.RegisterType<cp::SceneRenderPass>(std::string(SceneRenderPassTypeName));
		const bool negativeRegistered =
			renderPassRegistry.RegisterType<cp::NegativePostFX>(std::string(NegativePostFXTypeName));

		if (_context.mainLogger)
		{
			cp::ILogger& logger = *_context.mainLogger;
			logger.Log(CP_LOG_EVENT(cp::ILogger::Info, "ExamplePlugin", cp::Message::Create("Hello from plugin!")));
		}

		return sceneRegistered && negativeRegistered;
	}

	void ShutdownExamplePlugin(cp::PluginHostContext& _context)
	{
		if (_context.registryManager != nullptr)
		{
			if (cp::Registry<cp::IRenderPass>* renderPassRegistry =
				_context.registryManager->Find<cp::IRenderPass>(RenderPassRegistryName))
			{
				renderPassRegistry->Unregister(SceneRenderPassTypeName);
				renderPassRegistry->Unregister(NegativePostFXTypeName);
			}
		}

		if (_context.mainLogger)
		{
			cp::ILogger& logger = *_context.mainLogger;
			logger.Log(CP_LOG_EVENT(cp::ILogger::Info, "ExamplePlugin", cp::Message::Create("Goodbye from plugin!")));
		}

	}
}

CP_DECLARE_PLUGIN("ExamplePlugin", RegisterExamplePlugin, ShutdownExamplePlugin)

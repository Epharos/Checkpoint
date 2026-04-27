#include <Common/Core/Log.hpp>
#include <Common/Plugin/PluginAPI.hpp>

#include "RenderPasses/OpaqueMaterialPass.hpp"
#include "RenderPasses/NegativePFX.hpp"

#include "Components/RenderingComponentRegistrars.hpp"
#include "Systems/SpinSystem.hpp"

namespace
{
	inline constexpr std::string_view RenderPassRegistryName = "Renderpass";
	inline constexpr std::string_view EcsSystemRegistryName = "EcsSystem";
	inline constexpr std::string_view EcsComponentRegistryName = "EcsComponent";
	inline constexpr std::string_view OpaqueMaterialPassTypeName = "OpaqueMaterialPass";
	inline constexpr std::string_view NegativePostFXTypeName = "NegativePostFX";

	bool RegisterPasses(cp::Registry<cp::IRenderPass> &renderPassRegistry)
	{
		const bool opaqueRegistered =
			renderPassRegistry.RegisterType<cp::OpaqueMaterialPass>(
				std::string(OpaqueMaterialPassTypeName)
			);
		const bool negativeRegistered =
			renderPassRegistry.RegisterType<cp::NegativePostFX>(
				std::string(NegativePostFXTypeName)
			);

		return opaqueRegistered && negativeRegistered;
	}

	bool RegisterSystems(cp::Registry<cp::ecs::ISystem> &ecsSystemRegistry)
	{
		const bool spinSystemRegistered =
			ecsSystemRegistry.RegisterType<cp::SpinSystem>(
				cp::ecs::GuidToRegistryKey(cp::SpinSystemGuid)
			);

		return spinSystemRegistered;
	}

	bool RegisterComponents(cp::Registry<cp::ecs::IComponentRegistrar> &ecsComponentRegistry)
	{
		const bool transformComponentRegistered =
			ecsComponentRegistry.RegisterType<cp::Transform3DRegistrar>(
				cp::ecs::GuidToRegistryKey(cp::Transform3DGuid)
			);
		const bool cameraComponentRegistered =
			ecsComponentRegistry.RegisterType<cp::CameraRegistrar>(
				cp::ecs::GuidToRegistryKey(cp::CameraGuid)
			);
		const bool meshRendererComponentRegistered =
			ecsComponentRegistry.RegisterType<cp::MeshRendererRegistrar>(
				cp::ecs::GuidToRegistryKey(cp::MeshRendererGuid)
			);

		return transformComponentRegistered
			&& cameraComponentRegistered
			&& meshRendererComponentRegistered;
	}

	bool RegisterExamplePlugin(cp::PluginHostContext& _context)
	{
		if (_context.registryManager == nullptr || _context.assetRegistry == nullptr)
		{
			return false;
		}

		cp::SetPluginAssetRegistry(_context.assetRegistry);

		cp::Registry<cp::IRenderPass>& renderPassRegistry =
			_context.registryManager->GetOrCreate<cp::IRenderPass>(std::string(RenderPassRegistryName));
		cp::Registry<cp::ecs::ISystem>& ecsSystemRegistry =
			_context.registryManager->GetOrCreate<cp::ecs::ISystem>(std::string(EcsSystemRegistryName));
		cp::Registry<cp::ecs::IComponentRegistrar>& ecsComponentRegistry =
			_context.registryManager->GetOrCreate<cp::ecs::IComponentRegistrar>(std::string(EcsComponentRegistryName));

		const bool passRegistered = RegisterPasses(renderPassRegistry);
		const bool systemRegistered = RegisterSystems(ecsSystemRegistry);
		const bool componentRegistered = RegisterComponents(ecsComponentRegistry);

		if (_context.mainLogger)
		{
			cp::ILogger& logger = *_context.mainLogger;
			logger.Log(CP_LOG_EVENT(cp::ILogger::Info, "ExamplePlugin", cp::Message::Create("Hello from plugin!")));
		}

		return passRegistered && systemRegistered && componentRegistered;
	}

	void ShutdownExamplePlugin(cp::PluginHostContext& _context)
	{
		cp::SetPluginAssetRegistry(nullptr);

		if (_context.registryManager != nullptr)
		{
			if (cp::Registry<cp::IRenderPass>* renderPassRegistry =
				_context.registryManager->Find<cp::IRenderPass>(RenderPassRegistryName))
			{
				renderPassRegistry->Unregister(OpaqueMaterialPassTypeName);
				renderPassRegistry->Unregister(NegativePostFXTypeName);
			}

			if (cp::Registry<cp::ecs::ISystem>* ecsSystemRegistry =
				_context.registryManager->Find<cp::ecs::ISystem>(EcsSystemRegistryName))
			{
				ecsSystemRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::SpinSystemGuid));
			}

			if (cp::Registry<cp::ecs::IComponentRegistrar>* ecsComponentRegistry =
				_context.registryManager->Find<cp::ecs::IComponentRegistrar>(EcsComponentRegistryName))
			{
				ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::Transform3DGuid));
				ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::CameraGuid));
				ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::MeshRendererGuid));
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

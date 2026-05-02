#include <Common/Core/Log.hpp>
#include <Common/Plugin/PluginAPI.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>
#include <memory>

#include "RenderPasses/OpaqueMaterialPass.hpp"
#include "RenderPasses/NegativePFX.hpp"

#include "Components/Components.hpp"
#include "Systems/SpinSystem.hpp"

namespace
{
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
			ecsComponentRegistry.RegisterType<cp::TransformRegistrar>(
				cp::ecs::GuidToRegistryKey(cp::TransformGuid)
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
}

bool RegisterExamplePluginRuntime(cp::PluginRuntimeContext& _context)
{
	if (_context.registryManager == nullptr || _context.assetRegistry == nullptr)
	{
		return false;
	}

	// Create and install the component registration context
	static auto componentContext = std::make_unique<cp::ComponentRegistrationContext>(_context.assetRegistry);
	cp::SetComponentRegistrationContext(componentContext.get());

	cp::Registry<cp::IRenderPass>& renderPassRegistry =
		_context.registryManager->GetOrCreate<cp::IRenderPass>(std::string(cp::RenderPassRegistryName));
	cp::Registry<cp::ecs::ISystem>& ecsSystemRegistry =
		_context.registryManager->GetOrCreate<cp::ecs::ISystem>(std::string(cp::EcsSystemRegistryName));
	cp::Registry<cp::ecs::IComponentRegistrar>& ecsComponentRegistry =
		_context.registryManager->GetOrCreate<cp::ecs::IComponentRegistrar>(std::string(cp::EcsComponentRegistryName));

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

void ShutdownExamplePluginRuntime(cp::PluginRuntimeContext& _context)
{
	cp::SetComponentRegistrationContext(nullptr);

	if (_context.registryManager != nullptr)
	{
		if (cp::Registry<cp::IRenderPass>* renderPassRegistry =
			_context.registryManager->Find<cp::IRenderPass>(cp::RenderPassRegistryName))
		{
			renderPassRegistry->Unregister(OpaqueMaterialPassTypeName);
			renderPassRegistry->Unregister(NegativePostFXTypeName);
		}

		if (cp::Registry<cp::ecs::ISystem>* ecsSystemRegistry =
			_context.registryManager->Find<cp::ecs::ISystem>(cp::EcsSystemRegistryName))
		{
			ecsSystemRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::SpinSystemGuid));
		}

		if (cp::Registry<cp::ecs::IComponentRegistrar>* ecsComponentRegistry =
			_context.registryManager->Find<cp::ecs::IComponentRegistrar>(cp::EcsComponentRegistryName))
		{
			ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::TransformGuid));
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

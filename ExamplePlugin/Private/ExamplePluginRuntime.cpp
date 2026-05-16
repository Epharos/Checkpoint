#include <Common/Core/Log.hpp>
#include <Common/Plugin/PluginAPI.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>
#include <memory>

#include "ExamplePluginDir.hpp"
#include "RenderPasses/OpaqueMaterialPass.hpp"
#include "RenderPasses/NegativePFX.hpp"
#include "RenderPasses/SkyboxPass.hpp"

#include "Components/Components.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/ShooterSystem.hpp"
#include "Systems/SpinSystem.hpp"

namespace
{
	inline constexpr std::string_view OpaqueMaterialPassTypeName = "OpaqueMaterialPass";

	bool RegisterPasses(cp::Registry<cp::IRenderPass> &renderPassRegistry)
	{
		const bool opaqueRegistered =
			renderPassRegistry.RegisterType<cp::OpaqueMaterialPass>(
				std::string(OpaqueMaterialPassTypeName)
			);
		const bool negativeRegistered =
			renderPassRegistry.RegisterType<cp::NegativePostFX>(
				std::string(cp::NegativePostFXTypeName)
			);
		const bool skyboxRegistered =
			renderPassRegistry.RegisterType<cp::SkyboxPass>(
				std::string(cp::SkyboxPassTypeName)
			);
		return opaqueRegistered && negativeRegistered && skyboxRegistered;
	}

	bool RegisterSystems(cp::Registry<cp::ecs::ISystem> &ecsSystemRegistry)
	{
		const bool spinSystemRegistered =
			ecsSystemRegistry.RegisterType<cp::SpinSystem>(
				cp::ecs::GuidToRegistryKey(cp::SpinSystemGuid)
			);

		const bool shootSystemRegistered =
			ecsSystemRegistry.RegisterType<cp::ShooterSystem>(
				cp::ecs::GuidToRegistryKey(cp::ShooterSystemGuid)
			);

		const bool physicsSystemRegistered =
			ecsSystemRegistry.RegisterType<cp::PhysicsSystem>(
				cp::ecs::GuidToRegistryKey(cp::PhysicsSystemGuid)
			);

		return spinSystemRegistered && shootSystemRegistered && physicsSystemRegistered;
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
		const bool spawnerComponentRegistered =
			ecsComponentRegistry.RegisterType<cp::SpawnerRegistrar>(
				cp::ecs::GuidToRegistryKey(cp::SpawnerGuid)
			);
		const bool rigidbodyComponentRegistered =
			ecsComponentRegistry.RegisterType<cp::RigidBodyRegistrar>(
				cp::ecs::GuidToRegistryKey(cp::RigidBodyGuid)
			);

		return transformComponentRegistered
			&& cameraComponentRegistered
			&& meshRendererComponentRegistered
			&& spawnerComponentRegistered
			&& rigidbodyComponentRegistered;
	}
}

bool RegisterExamplePluginRuntime(cp::PluginRuntimeContext& _context)
{
	if (_context.registryManager == nullptr || _context.assetRegistry == nullptr)
	{
		return false;
	}

	cp::SetExamplePluginDir(_context.pluginDir);

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
			renderPassRegistry->Unregister(cp::NegativePostFXTypeName);
			renderPassRegistry->Unregister(cp::SkyboxPassTypeName);
		}

		if (cp::Registry<cp::ecs::ISystem>* ecsSystemRegistry =
			_context.registryManager->Find<cp::ecs::ISystem>(cp::EcsSystemRegistryName))
		{
			ecsSystemRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::SpinSystemGuid));
			ecsSystemRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::ShooterSystemGuid));
			ecsSystemRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::PhysicsSystemGuid));
		}

		if (cp::Registry<cp::ecs::IComponentRegistrar>* ecsComponentRegistry =
			_context.registryManager->Find<cp::ecs::IComponentRegistrar>(cp::EcsComponentRegistryName))
		{
			ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::TransformGuid));
			ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::CameraGuid));
			ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::MeshRendererGuid));
			ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::SpawnerGuid));
			ecsComponentRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::RigidBodyGuid));
		}
	}

	if (_context.mainLogger)
	{
		cp::ILogger& logger = *_context.mainLogger;
		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, "ExamplePlugin", cp::Message::Create("Goodbye from plugin!")));
	}
}

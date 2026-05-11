#include <Common/Plugin/PluginAPI.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>
#include <Common/Plugin/SystemAuthoring.hpp>
#include <Common/Plugin/RenderPassAuthoring.hpp>
#include <Common/Plugin/ViewportToolbarContribution.hpp>

#include "Components/Components.hpp"
#include "Components/TransformViewportToolbarContribution.hpp"
#include "Systems/PhysicsSystemAuthoring.hpp"
#include "Systems/SpinSystemAuthoring.hpp"
#include "RenderPasses/NegativePostFXAuthoring.hpp"
#include "RenderPasses/SkyboxAuthoring.hpp"

namespace
{
	bool RegisterEditorDeclarations(cp::RegistryManager& _registryManager)
	{
		cp::Registry<cp::IComponentAuthoring>& componentAuthoringRegistry =
			_registryManager.GetOrCreate<cp::IComponentAuthoring>(std::string(cp::EcsComponentAuthoringRegistryName));

		const bool transformAuthoringRegistered =
			componentAuthoringRegistry.RegisterType<cp::TransformAuthoring>(cp::ecs::GuidToRegistryKey(cp::TransformGuid));
		const bool cameraAuthoringRegistered =
			componentAuthoringRegistry.RegisterType<cp::CameraAuthoring>(cp::ecs::GuidToRegistryKey(cp::CameraGuid));
		const bool meshRendererAuthoringRegistered =
			componentAuthoringRegistry.RegisterType<cp::MeshRendererAuthoring>(cp::ecs::GuidToRegistryKey(cp::MeshRendererGuid));
		const bool spawnerAuthoringRegistered =
			componentAuthoringRegistry.RegisterType<cp::SpawnerAuthoring>(cp::ecs::GuidToRegistryKey(cp::SpawnerGuid));
		const bool rigidBodyAuthoringRegistered =
			componentAuthoringRegistry.RegisterType<cp::RigidBodyAuthoring>(cp::ecs::GuidToRegistryKey(cp::RigidBodyGuid));

		cp::Registry<cp::ISystemAuthoring>& systemAuthoringRegistry =
			_registryManager.GetOrCreate<cp::ISystemAuthoring>(std::string(cp::EcsSystemAuthoringRegistryName));

		const bool spinSystemAuthoringRegistered =
			systemAuthoringRegistry.RegisterType<cp::SpinSystemAuthoring>(cp::ecs::GuidToRegistryKey(cp::SpinSystemGuid));
		const bool physicsSystemAuthoringRegistered =
			systemAuthoringRegistry.RegisterType<cp::PhysicsSystemAuthoring>(cp::ecs::GuidToRegistryKey(cp::PhysicsSystemGuid));

		cp::Registry<cp::IRenderPassAuthoring>& renderPassAuthoringRegistry =
			_registryManager.GetOrCreate<cp::IRenderPassAuthoring>(std::string(cp::RenderPassAuthoringRegistryName));

		const bool negativePostFXAuthoringRegistered =
			renderPassAuthoringRegistry.RegisterType<cp::NegativePostFXAuthoring>(std::string(cp::NegativePostFXTypeName));
		const bool skyboxAuthoringRegistered =
			renderPassAuthoringRegistry.RegisterType<cp::SkyboxAuthoring>(std::string(cp::SkyboxPassTypeName));

		cp::Registry<cp::IViewportToolbarContribution>& viewportToolbarRegistry =
			_registryManager.GetOrCreate<cp::IViewportToolbarContribution>(std::string(cp::ViewportToolbarContributionRegistryName));

		const bool transformToolbarContributionRegistered =
			viewportToolbarRegistry.RegisterType<cp::TransformViewportToolbarContribution>(
				cp::ecs::GuidToRegistryKey(cp::TransformViewportToolbarContributionGuid));

		return transformAuthoringRegistered
			&& cameraAuthoringRegistered
			&& meshRendererAuthoringRegistered
			&& spawnerAuthoringRegistered
			&& rigidBodyAuthoringRegistered
			&& spinSystemAuthoringRegistered
			&& physicsSystemAuthoringRegistered
			&& negativePostFXAuthoringRegistered
			&& skyboxAuthoringRegistered
			&& transformToolbarContributionRegistered;
	}
}

bool RegisterExamplePluginEditor(cp::PluginEditorContext& _context)
{
	if (_context.registryManager == nullptr)
	{
		return false;
	}

	return RegisterEditorDeclarations(*_context.registryManager);
}

void ShutdownExamplePluginEditor(cp::PluginEditorContext& _context)
{
	if (_context.registryManager == nullptr)
	{
		return;
	}

	if (cp::Registry<cp::IComponentAuthoring>* componentAuthoringRegistry =
		_context.registryManager->Find<cp::IComponentAuthoring>(cp::EcsComponentAuthoringRegistryName))
	{
		componentAuthoringRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::TransformGuid));
		componentAuthoringRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::CameraGuid));
		componentAuthoringRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::MeshRendererGuid));
		componentAuthoringRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::SpawnerGuid));
		componentAuthoringRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::RigidBodyGuid));
	}

	if (cp::Registry<cp::ISystemAuthoring>* systemAuthoringRegistry =
		_context.registryManager->Find<cp::ISystemAuthoring>(cp::EcsSystemAuthoringRegistryName))
	{
		systemAuthoringRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::SpinSystemGuid));
		systemAuthoringRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::PhysicsSystemGuid));
	}

	if (cp::Registry<cp::IRenderPassAuthoring>* renderPassAuthoringRegistry =
		_context.registryManager->Find<cp::IRenderPassAuthoring>(cp::RenderPassAuthoringRegistryName))
	{
		renderPassAuthoringRegistry->Unregister(std::string(cp::NegativePostFXTypeName));
		renderPassAuthoringRegistry->Unregister(std::string(cp::SkyboxPassTypeName));
	}

	if (cp::Registry<cp::IViewportToolbarContribution>* viewportToolbarRegistry =
		_context.registryManager->Find<cp::IViewportToolbarContribution>(cp::ViewportToolbarContributionRegistryName))
	{
		viewportToolbarRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::TransformViewportToolbarContributionGuid));
	}
}

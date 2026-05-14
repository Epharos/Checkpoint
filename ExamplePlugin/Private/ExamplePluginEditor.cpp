#include <Common/Plugin/PluginAPI.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>
#include <Common/Plugin/SystemAuthoring.hpp>
#include <Common/Plugin/RenderPassAuthoring.hpp>
#include <Common/Plugin/ViewportToolbarContribution.hpp>

#include <Rendering/GizmoContribution.hpp>

#include "Components/Components.hpp"
#include "Components/TransformViewportToolbarContribution.hpp"
#include "Gizmo/TransformGizmoContribution.hpp"
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

		cp::Registry<cp::IGizmoContribution>& gizmoRegistry =
			_registryManager.GetOrCreate<cp::IGizmoContribution>(std::string(cp::GizmoContributionRegistryName));

		const bool transformGizmoRegistered =
			gizmoRegistry.RegisterType<cp::TransformGizmoContribution>(
				cp::ecs::GuidToRegistryKey(cp::TransformGizmoContributionGuid));

		return transformAuthoringRegistered
			&& cameraAuthoringRegistered
			&& meshRendererAuthoringRegistered
			&& spawnerAuthoringRegistered
			&& rigidBodyAuthoringRegistered
			&& spinSystemAuthoringRegistered
			&& physicsSystemAuthoringRegistered
			&& negativePostFXAuthoringRegistered
			&& skyboxAuthoringRegistered
			&& transformToolbarContributionRegistered
			&& transformGizmoRegistered;
	}
}

bool RegisterExamplePluginEditor(cp::PluginEditorContext& _context)
{
	if (_context.registryManager == nullptr)
	{
		return false;
	}

	if (!RegisterEditorDeclarations(*_context.registryManager))
	{
		return false;
	}

	if (_context.keybindRegistrar && _context.editorState && _context.viewportController)
	{
		_context.keybindRegistrar->Register(
			{
				.actionId = "viewport.focus_selected",
				.displayName = "Focus Selected",
				.category = "Viewport",
				.defaultChord = "F",
				.scope = "viewport"
			},
			[editorState = _context.editorState,
			 viewportCtrl = _context.viewportController]()
			{
				const cp::ecs::Entity* entity = editorState->GetSelectedEntity();
				if (entity == nullptr) return;

				cp::ecs::World* world = editorState->GetWorld();
				if (world == nullptr) return;

				if (!world->HasComponent<cp::Transform>(*entity)) return;

				const cp::Transform& t = world->GetComponent<cp::Transform>(*entity);
				viewportCtrl->FocusOn(t.x, t.y, t.z, 5.f);
			});

		_context.viewportController->SetOrbitTargetProvider(
			[editorState = _context.editorState](float& _x, float& _y, float& _z) -> bool
			{
				const cp::ecs::Entity* entity = editorState->GetSelectedEntity();
				if (entity == nullptr) return false;

				cp::ecs::World* world = editorState->GetWorld();
				if (world == nullptr) return false;

				if (!world->HasComponent<cp::Transform>(*entity)) return false;

				const cp::Transform& t = world->GetComponent<cp::Transform>(*entity);
				_x = t.x;
				_y = t.y;
				_z = t.z;
				return true;
			});
	}

	return true;
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

	if (cp::Registry<cp::IGizmoContribution>* gizmoRegistry =
		_context.registryManager->Find<cp::IGizmoContribution>(cp::GizmoContributionRegistryName))
	{
		gizmoRegistry->Unregister(cp::ecs::GuidToRegistryKey(cp::TransformGizmoContributionGuid));
	}

	if (_context.keybindRegistrar)
	{
		_context.keybindRegistrar->Unregister("viewport.focus_selected");
	}

	if (_context.viewportController)
	{
		_context.viewportController->SetOrbitTargetProvider(nullptr);
	}
}

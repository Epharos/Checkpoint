#include <Common/Plugin/PluginAPI.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>

#include "Components/Components.hpp"

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

		return transformAuthoringRegistered
			&& cameraAuthoringRegistered
			&& meshRendererAuthoringRegistered;
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
	}
}

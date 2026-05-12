#pragma once

#include <string_view>

namespace cp
{
	inline constexpr std::string_view RenderPassRegistryName = "Renderpass";
	inline constexpr std::string_view EcsSystemRegistryName = "EcsSystem";
	inline constexpr std::string_view EcsComponentRegistryName = "EcsComponent";
	inline constexpr std::string_view EcsComponentAuthoringRegistryName = "EcsComponentAuthoring";
	inline constexpr std::string_view EcsSystemAuthoringRegistryName = "EcsSystemAuthoring";
	inline constexpr std::string_view RenderPassAuthoringRegistryName = "RenderPassAuthoring";
	inline constexpr std::string_view ViewportToolbarContributionRegistryName = "ViewportToolbarContribution";
	inline constexpr std::string_view GizmoContributionRegistryName = "GizmoContribution";
}

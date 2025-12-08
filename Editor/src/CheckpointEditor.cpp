#include "pch.hpp"
#include "CheckpointEditor.hpp"

cp::VulkanContext cp::CheckpointEditor::VulkanCtx = {};
cp::Project cp::CheckpointEditor::CurrentProject = {};
cp::SceneAsset* cp::CheckpointEditor::CurrentScene = new cp::SceneAsset;
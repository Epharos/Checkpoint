#pragma once

#include "../../pch.hpp"
#include "../../Resources/Mesh.hpp"
#include "../../Resources/Material.hpp"

struct RenderCommand
{
	Resource::Mesh* mesh;
	Render::Material* material;
	glm::mat4 modelMatrix;
};
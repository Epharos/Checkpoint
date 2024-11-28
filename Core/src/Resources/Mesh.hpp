#pragma once

#include "../pch.hpp"

namespace Resource
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	class Mesh
	{
	protected:
		std::vector<Vertex> vertices;
		std::vector<int> indices;

	public:
		Mesh() = default;
		Mesh(const std::vector<Vertex>& vertices, const std::vector<int>& indices)
			: vertices(vertices), indices(indices) {}

		inline constexpr const std::vector<Vertex>& GetVertices() const { return vertices; }
		inline constexpr const std::vector<int>& GetIndices() const { return indices; }
	};
}
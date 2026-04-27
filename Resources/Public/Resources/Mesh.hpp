#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace cp
{
	struct MeshVertex
	{
		float position[3] = { 0.f, 0.f, 0.f };
		float normal[3] = { 0.f, 0.f, 0.f };
		float texCoord[2] = { 0.f, 0.f };
		float tangent[3] = { 0.f, 0.f, 0.f };
		float bitangent[3] = { 0.f, 0.f, 0.f };
	};

	class Mesh
	{
	public:
		[[nodiscard]] const std::vector<MeshVertex>& GetVertices() const
		{
			return vertices;
		}

		[[nodiscard]] const std::vector<uint32_t>& GetIndices() const
		{
			return indices;
		}

		[[nodiscard]] bool IsValid() const
		{
			return !vertices.empty() && !indices.empty();
		}

		void SetData(std::vector<MeshVertex> _vertices, std::vector<uint32_t> _indices)
		{
			vertices = std::move(_vertices);
			indices = std::move(_indices);
		}

	private:
		std::vector<MeshVertex> vertices;
		std::vector<uint32_t> indices;
	};
}

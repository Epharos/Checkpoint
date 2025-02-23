#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

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
		std::vector<uint32_t> indices;

		vk::Buffer vertexBuffer;
		vk::DeviceMemory vertexBufferMemory;
		vk::Buffer indexBuffer;
		vk::DeviceMemory indexBufferMemory;

		const Context::VulkanContext* context;

	public:
		Mesh(const Context::VulkanContext& _context, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		~Mesh();

		inline constexpr const std::vector<Vertex>& GetVertices() const { return vertices; }
		inline constexpr const std::vector<uint32_t>& GetIndices() const { return indices; }
		inline constexpr const uint32_t GetIndexCount() const { return static_cast<const uint32_t>(indices.size()); }

		inline constexpr vk::Buffer GetVertexBuffer() const { return vertexBuffer; }
		inline constexpr vk::Buffer& GetVertexBuffer() { return vertexBuffer; }
		inline constexpr vk::DeviceMemory GetVertexBufferMemory() const { return vertexBufferMemory; }
		inline constexpr vk::Buffer GetIndexBuffer() const { return indexBuffer; }
		inline constexpr vk::DeviceMemory GetIndexBufferMemory() const { return indexBufferMemory; }

		static Mesh* LoadMesh(const Context::VulkanContext& _context, const std::string& _path);
	};
}
#include "pch.hpp"
#include "Mesh.hpp"

Resource::Mesh::Mesh(const Context::VulkanContext& _context, std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
{
	this->vertices = _vertices;
	this->indices = _indices;
	this->context = &_context;

	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		Vertex& v0 = vertices[_indices[i + 0]];
		Vertex& v1 = vertices[_indices[i + 1]];
		Vertex& v2 = vertices[_indices[i + 2]];

		glm::vec3 edge1 = v1.position - v0.position;
		glm::vec3 edge2 = v2.position - v0.position;

		glm::vec2 deltaUV1 = v1.uv - v0.uv;
		glm::vec2 deltaUV2 = v2.uv - v0.uv;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		glm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent = glm::normalize(tangent);

		glm::vec3 bitangent;
		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent = glm::normalize(bitangent);

		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;

		v0.bitangent += bitangent;
		v1.bitangent += bitangent;
		v2.bitangent += bitangent;

		LOG_DEBUG(MF("Calculated Tangeant and Bitangeant ", 
			tangent.x, ";", tangent.y, ";", tangent.z, " | ", 
			bitangent.x, ";", bitangent.y, ";", bitangent.z));
	}

	vk::DeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();
	vk::DeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();

	vk::Buffer stagingVertexBuffer;
	vk::DeviceMemory stagingVertexBufferMemory;
	vk::Buffer stagingIndexBuffer;
	vk::DeviceMemory stagingIndexBufferMemory;

	stagingVertexBuffer = Helper::Memory::CreateBuffer(_context.GetDevice(), _context.GetPhysicalDevice(), vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, stagingVertexBufferMemory);
	stagingIndexBuffer = Helper::Memory::CreateBuffer(_context.GetDevice(), _context.GetPhysicalDevice(), indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, stagingIndexBufferMemory);

	Helper::Memory::MapMemory(_context.GetDevice(), stagingVertexBufferMemory, vertexBufferSize, vertices.data());
	Helper::Memory::MapMemory(_context.GetDevice(), stagingIndexBufferMemory, indexBufferSize, indices.data());

	vertexBuffer = Helper::Memory::CreateBuffer(_context.GetDevice(), _context.GetPhysicalDevice(), vertexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBufferMemory);
	indexBuffer = Helper::Memory::CreateBuffer(_context.GetDevice(), _context.GetPhysicalDevice(), indexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBufferMemory);

	Helper::Memory::CopyBuffer(_context.GetDevice(), _context.GetCommandPool(), _context.GetDevice().getQueue(_context.GetQueueFamilyIndices().graphicsFamily.value(), 0), stagingVertexBuffer, vertexBuffer, vertexBufferSize);
	Helper::Memory::CopyBuffer(_context.GetDevice(), _context.GetCommandPool(), _context.GetDevice().getQueue(_context.GetQueueFamilyIndices().graphicsFamily.value(), 0), stagingIndexBuffer, indexBuffer, indexBufferSize);

	Helper::Memory::DestroyBuffer(_context.GetDevice(), stagingVertexBuffer, stagingVertexBufferMemory);
	Helper::Memory::DestroyBuffer(_context.GetDevice(), stagingIndexBuffer, stagingIndexBufferMemory);
}

Resource::Mesh::~Mesh()
{
	context->GetDevice().waitIdle();
	Helper::Memory::DestroyBuffer(context->GetDevice(), vertexBuffer, vertexBufferMemory);
	Helper::Memory::DestroyBuffer(context->GetDevice(), indexBuffer, indexBufferMemory);
}

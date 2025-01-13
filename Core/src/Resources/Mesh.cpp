#include "pch.hpp"
#include "Mesh.hpp"

Resource::Mesh::Mesh(const Context::VulkanContext& _context, const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
{
	this->vertices = _vertices;
	this->indices = _indices;
	this->context = &_context;

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

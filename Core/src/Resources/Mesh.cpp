#include "pch.hpp"
#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

std::shared_ptr<Resource::Mesh> Resource::Mesh::LoadMesh(const Context::VulkanContext& _context, const std::string& _path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(_path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG_ERROR("Failed to load model: " + std::string(importer.GetErrorString()));
		throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
		return nullptr;
	}

	aiMesh* mesh = scene->mMeshes[0];

	std::vector<Resource::Vertex> vertices;
	std::vector<uint32_t> indexes;

	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		Resource::Vertex vertex;
		vertex.position = { mesh->mVertices[i].x, -mesh->mVertices[i].y, -mesh->mVertices[i].z };
		vertex.normal = { mesh->mNormals[i].x, -mesh->mNormals[i].y, -mesh->mNormals[i].z };
		vertex.tangent = { mesh->mTangents[i].x, -mesh->mTangents[i].y, -mesh->mTangents[i].z };
		vertex.bitangent = { mesh->mBitangents[i].x, -mesh->mBitangents[i].y, -mesh->mBitangents[i].z };

		if (mesh->mTextureCoords[0])
		{
			vertex.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertex.uv = { 0.0f, 0.0f };
		}

		vertices.push_back(vertex);
	}

	for (uint32_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			indexes.push_back(face.mIndices[j]);
		}
	}

	LOG_INFO("Loaded mesh: " + _path + " with " + std::to_string(vertices.size()) + " vertices and " + std::to_string(indexes.size()) + " indexes");

	return std::make_shared<Resource::Mesh>(_context, vertices, indexes);
}

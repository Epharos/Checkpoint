#include "../../Public/Resources/MeshLoader.hpp"

#include <RHI/RenderingHardwareInterface.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <limits>
#include <utility>

namespace cp
{
	std::shared_ptr<Mesh> AssetLoader<Mesh>::Load(
		const std::filesystem::path& _path,
		[[maybe_unused]] RenderingHardwareInterface& _rhi)
	{
		Assimp::Importer importer;

		constexpr unsigned int flags =
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ImproveCacheLocality |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_SortByPType;

		const aiScene* scene = importer.ReadFile(_path.string(), flags);
		if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 || scene->mRootNode == nullptr)
		{
			return nullptr;
		}

		auto mesh = std::make_shared<Mesh>();
		std::vector<MeshVertex> vertices;
		std::vector<uint32_t> indices;

		for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* aiMeshPtr = scene->mMeshes[meshIndex];
			if (aiMeshPtr == nullptr || !aiMeshPtr->HasPositions())
			{
				continue;
			}

			const size_t baseVertex = vertices.size();
			vertices.reserve(vertices.size() + aiMeshPtr->mNumVertices);

			for (unsigned int vertexIndex = 0; vertexIndex < aiMeshPtr->mNumVertices; ++vertexIndex)
			{
				MeshVertex vertex{};

				const aiVector3D& position = aiMeshPtr->mVertices[vertexIndex];
				vertex.position[0] = position.x;
				vertex.position[1] = position.y;
				vertex.position[2] = position.z;

				if (aiMeshPtr->HasNormals())
				{
					const aiVector3D& normal = aiMeshPtr->mNormals[vertexIndex];
					vertex.normal[0] = normal.x;
					vertex.normal[1] = normal.y;
					vertex.normal[2] = normal.z;
				}

				if (aiMeshPtr->HasTextureCoords(0))
				{
					const aiVector3D& uv = aiMeshPtr->mTextureCoords[0][vertexIndex];
					vertex.texCoord[0] = uv.x;
					vertex.texCoord[1] = uv.y;
				}

				if (aiMeshPtr->HasTangentsAndBitangents())
				{
					const aiVector3D& tangent = aiMeshPtr->mTangents[vertexIndex];
					const aiVector3D& bitangent = aiMeshPtr->mBitangents[vertexIndex];
					vertex.tangent[0] = tangent.x;
					vertex.tangent[1] = tangent.y;
					vertex.tangent[2] = tangent.z;
					vertex.bitangent[0] = bitangent.x;
					vertex.bitangent[1] = bitangent.y;
					vertex.bitangent[2] = bitangent.z;
				}

				vertices.push_back(vertex);
			}

			indices.reserve(indices.size() + aiMeshPtr->mNumFaces * 3);
			for (unsigned int faceIndex = 0; faceIndex < aiMeshPtr->mNumFaces; ++faceIndex)
			{
				const aiFace& face = aiMeshPtr->mFaces[faceIndex];
				if (face.mNumIndices != 3)
				{
					continue;
				}

				for (unsigned int index = 0; index < face.mNumIndices; ++index)
				{
					const size_t mergedIndex = baseVertex + static_cast<size_t>(face.mIndices[index]);
					if (mergedIndex > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
					{
						return nullptr;
					}

					indices.push_back(static_cast<uint32_t>(mergedIndex));
				}
			}
		}

		if (vertices.empty() || indices.empty())
		{
			return nullptr;
		}

		mesh->SetData(std::move(vertices), std::move(indices));
		return mesh;
	}
}

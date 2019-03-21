#include "JWAssimpLoader.h"

using namespace JWEngine;

inline auto ConvertAiVector3DToXMFLOAT2(aiVector3D input)->XMFLOAT2
{
	return XMFLOAT2(input.x, input.y);
}

inline auto ConvertAiVector3DToXMFLOAT3(aiVector3D input)->XMFLOAT3
{
	return XMFLOAT3(input.x, input.y, input.z);
}

auto JWAssimpLoader::LoadObj(STRING Directory, STRING ModelFileName) noexcept->SModelData
{
	SModelData temporary_model_data{};

	Assimp::Importer m_AssimpImporter{};
	const aiScene* m_AssimpScene{ m_AssimpImporter.ReadFile(Directory + ModelFileName, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded) };

	if (m_AssimpScene)
	{
		if (m_AssimpScene->HasMaterials())
		{
			auto material_count{ m_AssimpScene->mNumMaterials };
			auto properties_count{ m_AssimpScene->mMaterials[0]->mNumProperties };

			aiString path{};
			m_AssimpScene->mMaterials[material_count - 1]->GetTexture(aiTextureType::aiTextureType_AMBIENT, 0, &path);
			STRING path_string{ Directory + path.C_Str() };
			
			if (path.length == 0)
			{
				temporary_model_data.HasTexture = false;
			}
			else
			{
				temporary_model_data.HasTexture = true;
				temporary_model_data.TextureFileNameW = StringToWstring(path_string);
			}
		}

		if (m_AssimpScene->HasMeshes())
		{
			auto mesh_count{ m_AssimpScene->mNumMeshes };
			size_t mesh_index{};

			if (m_AssimpScene->mMeshes[mesh_index]->HasPositions())
			{
				size_t vertices_count{ m_AssimpScene->mMeshes[mesh_index]->mNumVertices };

				XMFLOAT3 position{};
				XMFLOAT2 texcoord{};
				XMFLOAT3 normal{};

				for (size_t iterator_vertices{}; iterator_vertices < vertices_count; ++iterator_vertices)
				{
					position = ConvertAiVector3DToXMFLOAT3(m_AssimpScene->mMeshes[mesh_index]->mVertices[iterator_vertices]);
					texcoord = ConvertAiVector3DToXMFLOAT2(m_AssimpScene->mMeshes[mesh_index]->mTextureCoords[0][iterator_vertices]);
					normal = ConvertAiVector3DToXMFLOAT3(m_AssimpScene->mMeshes[mesh_index]->mNormals[iterator_vertices]);

					temporary_model_data.VertexData.Vertices.emplace_back(position, texcoord, normal);
				}
			}
			else
			{
				JWAbort("Loaded model doesn't have positions");
			}

			if (m_AssimpScene->mMeshes[mesh_index]->HasFaces())
			{
				size_t faces_count{ m_AssimpScene->mMeshes[mesh_index]->mNumFaces };

				for (size_t iterator_faces{}; iterator_faces < faces_count; ++iterator_faces)
				{
					auto& indices_count = m_AssimpScene->mMeshes[mesh_index]->mFaces[iterator_faces].mNumIndices;
					auto& indices = m_AssimpScene->mMeshes[mesh_index]->mFaces[iterator_faces].mIndices;

					if (indices_count == 3)
					{
						temporary_model_data.IndexData.Indices.emplace_back(indices[0], indices[1], indices[2]);
					}
					else
					{
						JWAbort("Index count is not 3");
					}
				}
			}
			else
			{
				JWAbort("Loaded model doesn't have faces");
			}
		}
	}
	else
	{
		JWAbort("Model file not loaded.");
	}

	return temporary_model_data;
}
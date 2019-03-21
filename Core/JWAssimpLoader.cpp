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

inline auto ConvertAiColor4DToXMFLOAT4(aiColor4D input)->XMFLOAT4
{
	return XMFLOAT4(input.r, input.g, input.b, input.a);
}

auto JWAssimpLoader::LoadObj(STRING Directory, STRING ModelFileName) noexcept->SModelData
{
	SModelData temporary_model_data{};

	Assimp::Importer m_AssimpImporter{};
	const aiScene* m_AssimpScene{ m_AssimpImporter.ReadFile(Directory + ModelFileName, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded) };

	if (m_AssimpScene && m_AssimpScene->HasMeshes())
	{
		auto mesh_count{ m_AssimpScene->mNumMeshes };
		unsigned int indices_offset{};

		for (unsigned int mesh_index{}; mesh_index < mesh_count; ++mesh_index)
		{
			auto material = m_AssimpScene->mMaterials[m_AssimpScene->mMeshes[mesh_index]->mMaterialIndex];
			aiColor4D diffuse{};
			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);

			if (!temporary_model_data.HasTexture)
			{
				aiString path{};
				aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &path);
				STRING path_string{ Directory + path.C_Str() };
				if (path.length)
				{
					temporary_model_data.HasTexture = true;
					temporary_model_data.TextureFileNameW = StringToWstring(path_string);
				}
			}

			if (m_AssimpScene->mMeshes[mesh_index]->HasPositions())
			{
				size_t mesh_vertices_count{ m_AssimpScene->mMeshes[mesh_index]->mNumVertices };

				XMFLOAT3 position{};
				XMFLOAT2 texcoord{};
				XMFLOAT3 normal{};
				XMFLOAT4 color_diffuse{ ConvertAiColor4DToXMFLOAT4(diffuse) };

				for (size_t iterator_vertices{}; iterator_vertices < mesh_vertices_count; ++iterator_vertices)
				{
					position = ConvertAiVector3DToXMFLOAT3(m_AssimpScene->mMeshes[mesh_index]->mVertices[iterator_vertices]);
					texcoord = ConvertAiVector3DToXMFLOAT2(m_AssimpScene->mMeshes[mesh_index]->mTextureCoords[0][iterator_vertices]);
					normal = ConvertAiVector3DToXMFLOAT3(m_AssimpScene->mMeshes[mesh_index]->mNormals[iterator_vertices]);

					temporary_model_data.VertexData.Vertices.emplace_back(position, texcoord, normal, color_diffuse);
				}
			}
			else
			{
				JWAbort("Loaded model doesn't have positions");
			}

			if (m_AssimpScene->mMeshes[mesh_index]->HasFaces())
			{
				size_t faces_count{ m_AssimpScene->mMeshes[mesh_index]->mNumFaces };
				unsigned int last_index{};

				for (size_t iterator_faces{}; iterator_faces < faces_count; ++iterator_faces)
				{
					auto& indices_count = m_AssimpScene->mMeshes[mesh_index]->mFaces[iterator_faces].mNumIndices;
					auto& indices = m_AssimpScene->mMeshes[mesh_index]->mFaces[iterator_faces].mIndices;

					if (indices_count == 3)
					{
						temporary_model_data.IndexData.Indices.emplace_back(
							indices_offset + indices[0],
							indices_offset + indices[1],
							indices_offset + indices[2]);
					}
					else
					{
						JWAbort("Index count is not 3");
					}

					last_index = max(last_index, indices[0]);
					last_index = max(last_index, indices[1]);
					last_index = max(last_index, indices[2]);
				}

				indices_offset += last_index + 1;
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
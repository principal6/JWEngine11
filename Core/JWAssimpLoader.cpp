#include "JWAssimpLoader.h"

using namespace JWEngine;

auto JWAssimpLoader::LoadStaticModel(STRING Directory, STRING ModelFileName) noexcept->SStaticModelData
{
	SStaticModelData temporary_model_data{};

	Assimp::Importer assimp_importer{};
	const aiScene* assimp_scene{ assimp_importer.ReadFile(Directory + ModelFileName,
		aiProcess_ConvertToLeftHanded | aiProcess_ValidateDataStructure | aiProcess_OptimizeMeshes |
		aiProcess_PreTransformVertices | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices) };

	if (assimp_scene && assimp_scene->HasMeshes())
	{
		auto mesh_count{ assimp_scene->mNumMeshes };
		unsigned int indices_offset{};

		for (unsigned int mesh_index{}; mesh_index < mesh_count; ++mesh_index)
		{
			auto material = assimp_scene->mMaterials[assimp_scene->mMeshes[mesh_index]->mMaterialIndex];
			aiColor4D ai_diffuse{};
			aiColor4D ai_specular{};
			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &ai_diffuse);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &ai_specular);

			// Load the texture only once per model (not per mesh).
			// i.e. multi-texture not supported.
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

			if (assimp_scene->mMeshes[mesh_index]->HasPositions())
			{
				size_t mesh_vertices_count{ assimp_scene->mMeshes[mesh_index]->mNumVertices };

				XMFLOAT3 position{};
				XMFLOAT2 texcoord{};
				XMFLOAT3 normal{};
				
				XMFLOAT4 diffuse{ ConvertaiColor4DToXMFLOAT4(ai_diffuse) };
				XMFLOAT4 specular{ ConvertaiColor4DToXMFLOAT4(ai_specular) };

				for (size_t iterator_vertices{}; iterator_vertices < mesh_vertices_count; ++iterator_vertices)
				{
					position = ConvertaiVector3DToXMFLOAT3(assimp_scene->mMeshes[mesh_index]->mVertices[iterator_vertices]);
					
					if (assimp_scene->mMeshes[mesh_index]->mTextureCoords[0] == nullptr)
					{
						// No texture coordinates in model file
						texcoord = XMFLOAT2(0, 0);
					}
					else
					{
						texcoord = ConvertaiVector3DToXMFLOAT2(assimp_scene->mMeshes[mesh_index]->mTextureCoords[0][iterator_vertices]);
					}

					normal = ConvertaiVector3DToXMFLOAT3(assimp_scene->mMeshes[mesh_index]->mNormals[iterator_vertices]);

					temporary_model_data.VertexData.Vertices.emplace_back(position, texcoord, normal, diffuse, specular);
				}
			}
			else
			{
				JWAbort("Loaded model doesn't have positions");
			}

			if (assimp_scene->mMeshes[mesh_index]->HasFaces())
			{
				size_t faces_count{ assimp_scene->mMeshes[mesh_index]->mNumFaces };
				unsigned int last_index{};

				for (size_t iterator_faces{}; iterator_faces < faces_count; ++iterator_faces)
				{
					auto& indices_count = assimp_scene->mMeshes[mesh_index]->mFaces[iterator_faces].mNumIndices;
					auto& indices = assimp_scene->mMeshes[mesh_index]->mFaces[iterator_faces].mIndices;

					if (indices_count == 3)
					{
						temporary_model_data.IndexData.Indices.emplace_back(
							indices_offset + indices[0],
							indices_offset + indices[1],
							indices_offset + indices[2]);
					}
					else
					{
						//JWAbort("Index count is not 3");
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
		JWAbort("Model file not found.");
	}

	return temporary_model_data;
}

auto JWAssimpLoader::LoadRiggedModel(STRING Directory, STRING ModelFileName) noexcept->SSkinnedModelData
{
	SSkinnedModelData result{};
	result.BaseDirectory = Directory;

	Assimp::Importer importer{};

	// aiProcess_FindDegenerates (this may cause indices count to be 2, not 3)

	const aiScene* scene{ importer.ReadFile(result.BaseDirectory + ModelFileName,
		aiProcess_ConvertToLeftHanded | aiProcess_ValidateDataStructure | aiProcess_OptimizeMeshes |
		aiProcess_SplitLargeMeshes | aiProcess_ImproveCacheLocality |
		aiProcess_Triangulate | aiProcess_SplitByBoneCount | aiProcess_JoinIdenticalVertices) };

	if (scene == nullptr)
	{
		JWAbort("Model not imported. Check out the model file.");
	}
	else if (scene->mRootNode == nullptr)
	{
		JWAbort("Model not imported. Check out the model file.");
	}

	// Extract node hierarchy from model file.
	ExtractNodeTree(scene, scene->mRootNode, -1, result.NodeTree);
	
	// Build meshes and bones from nodes in extracted node hierarchy.
	BuildMeshesAndBonesFromNodes(scene, result);

	// Match bones and vertices
	MatchBonesAndVertices(result.BoneTree, result.VertexData);

	// Match bones and nodes
	MatchBonesAndNodes(result.BoneTree, result.NodeTree);

	// Extract animations
	ExtractAnimationSet(scene, result.NodeTree, result.AnimationSet);

	return result;
}

PRIVATE void JWAssimpLoader::ExtractNodeTree(const aiScene* Scene, const aiNode* Node, int ParentNodeID, SModelNodeTree& OutNodeTree) noexcept
{
	OutNodeTree.vNodes.push_back(SModelNode());
	int new_node_id = static_cast<int>(OutNodeTree.vNodes.size() - 1);
	SModelNode& new_node = OutNodeTree.vNodes[new_node_id];
	
	// Set node's ID
	new_node.ID = new_node_id;

	// Set node's parent ID
	new_node.ParentID = ParentNodeID;

	// Set node's name
	new_node.Name = Node->mName.C_Str();

	// Set node's transformation matrix
	new_node.Transformation = ConvertaiMatrix4x4ToXMMATRIX(Node->mTransformation);

	// Set mehses' ID
	if (Node->mNumMeshes)
	{
		for (unsigned int i = 0; i < Node->mNumMeshes; ++i)
		{
			new_node.vMeshesID.push_back(Node->mMeshes[i]);

			new_node.AccumulatedVerticesCount = OutNodeTree.TotalVerticesCount;

			OutNodeTree.TotalVerticesCount += Scene->mMeshes[Node->mMeshes[i]]->mNumVertices;
		}
	}

	// @warning: Node's bone ID (new_node.BoneID) will be set later by MatchBonesAndNodes()

	// Extract child nodes
	if (Node->mNumChildren)
	{
		int this_node_id = new_node.ID;

		for (unsigned int i = 0; i < Node->mNumChildren; ++i)
		{
			ExtractNodeTree(Scene, Node->mChildren[i], this_node_id, OutNodeTree);
		}

		for (const auto& iter_node : OutNodeTree.vNodes)
		{
			if (iter_node.ParentID == this_node_id)
			{
				OutNodeTree.vNodes[this_node_id].vChildrenID.push_back(iter_node.ID);
			}
		}
	}
}

PRIVATE void JWAssimpLoader::BuildMeshesAndBonesFromNodes(const aiScene* Scene, SSkinnedModelData& OutModelData) noexcept
{
	int indices_offset{};

	// Iterate each node in node tree
	for (const auto& node : OutModelData.NodeTree.vNodes)
	{
		// Get all meshes' id in this node
		for (const auto mesh_id : node.vMeshesID)
		{
			auto ai_mesh = Scene->mMeshes[mesh_id];
			auto ai_material = Scene->mMaterials[ai_mesh->mMaterialIndex];

			aiColor4D ai_diffuse{};
			aiColor4D ai_specular{};
			aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_DIFFUSE, &ai_diffuse);
			aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_SPECULAR, &ai_specular);

			// Load the texture only once per model (not per mesh).
			// i.e. multi-texture not supported.
			if (!OutModelData.HasTexture)
			{
				aiString path{};
				aiGetMaterialTexture(ai_material, aiTextureType_DIFFUSE, 0, &path);
				STRING path_string{ OutModelData.BaseDirectory + path.C_Str() };
				if (path.length)
				{
					OutModelData.HasTexture = true;
					OutModelData.TextureFileNameW = StringToWstring(path_string);
				}
			}

			if (ai_mesh->HasPositions())
			{
				XMFLOAT3 position{};
				XMFLOAT3 normal{};
				XMFLOAT2 texcoord{};

				XMFLOAT4 diffuse{ ConvertaiColor4DToXMFLOAT4(ai_diffuse) };
				XMFLOAT4 specular{ ConvertaiColor4DToXMFLOAT4(ai_specular) };

				for (unsigned int vertex_id = 0; vertex_id < ai_mesh->mNumVertices; ++vertex_id)
				{
					position = ConvertaiVector3DToXMFLOAT3(ai_mesh->mVertices[vertex_id]);
					normal = ConvertaiVector3DToXMFLOAT3(ai_mesh->mNormals[vertex_id]);
					texcoord = ConvertaiVector3DToXMFLOAT2(ai_mesh->mTextureCoords[0][vertex_id]);

					OutModelData.VertexData.Vertices.emplace_back(position, texcoord, normal, diffuse, specular);
				}
			}
			else
			{
				JWAbort("Loaded model doesn't have positions");
			}

			if (ai_mesh->HasFaces())
			{
				unsigned int last_index{};
				for (unsigned int iterator_faces{}; iterator_faces < ai_mesh->mNumFaces; ++iterator_faces)
				{
					auto& indices_count = ai_mesh->mFaces[iterator_faces].mNumIndices;
					auto& indices = ai_mesh->mFaces[iterator_faces].mIndices;

					if (indices_count == 3)
					{
						OutModelData.IndexData.Indices.emplace_back(
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

			// If this mesh refers to bones
			if (ai_mesh->mNumBones)
			{
				// Iterate each bone
				for (unsigned int i = 0; i < ai_mesh->mNumBones; ++i)
				{
					ExtractBone(ai_mesh->mBones[i], node.AccumulatedVerticesCount, OutModelData.BoneTree);
				}
			}
		}
	}
}

PRIVATE void JWAssimpLoader::ExtractBone(const aiBone* paiBone, int VertexOffset, SModelBoneTree& OutBoneTree) noexcept
{
	bool IsNewBone{ true };
	int bone_id{};

	for (const auto& bone : OutBoneTree.vBones)
	{
		if (bone.Name.compare(paiBone->mName.C_Str()) == 0)
		{
			// @important
			// Avoid duplicate bone creation.
			// This bone is already in bone tree.

			IsNewBone = false;
			bone_id = bone.ID;

			break;
		}
	}

	if (IsNewBone)
	{
		OutBoneTree.vBones.push_back(SModelBone());
		bone_id = static_cast<int>(OutBoneTree.vBones.size() - 1);
	}

	auto& bone = OutBoneTree.vBones[bone_id];

	// Set bone's id
	bone.ID = bone_id;

	// Set bone's name
	bone.Name = paiBone->mName.C_Str();

	// Set bone's offset matrix
	bone.Offset = ConvertaiMatrix4x4ToXMMATRIX(paiBone->mOffsetMatrix);

	// Set bone's weights array
	if (paiBone->mNumWeights)
	{
		for (unsigned int i = 0; i < paiBone->mNumWeights; ++i)
		{
			bone.vWeights.push_back(SModelWeight());
			int new_weight_id = static_cast<int>(bone.vWeights.size() - 1);
			auto& new_weight = bone.vWeights[new_weight_id];

			// @important!!
			// VertexID needs to be offset.
			new_weight.VertexID = paiBone->mWeights[i].mVertexId + VertexOffset;

			new_weight.Weight = paiBone->mWeights[i].mWeight;
		}
	}
}

PRIVATE void JWAssimpLoader::MatchBonesAndVertices(const SModelBoneTree& BoneTree, SSkinnedVertexData& OutVertexData) noexcept
{
	for (const auto& bone : BoneTree.vBones)
	{
		for (const auto& weight : bone.vWeights)
		{
			OutVertexData.Vertices[weight.VertexID].AddBone(bone.ID, weight.Weight);
		}
	}
}

PRIVATE void JWAssimpLoader::MatchBonesAndNodes(const SModelBoneTree& BoneTree, SModelNodeTree& OutNodeTree) noexcept
{
	for (auto& node : OutNodeTree.vNodes)
	{
		for (const auto& bone : BoneTree.vBones)
		{
			if (node.Name.compare(bone.Name) == 0)
			{
				// We found the matching bone of this node.
				node.BoneID = bone.ID;
				break;
			}
		}
	}
}

PRIVATE void JWAssimpLoader::ExtractAnimationSet(const aiScene* Scene, const SModelNodeTree& NodeTree, SModelAnimationSet& OutAnimationSet)
{
	if (Scene->mNumAnimations)
	{
		for (unsigned int animation_id = 0; animation_id < Scene->mNumAnimations; ++animation_id)
		{
			OutAnimationSet.vAnimations.push_back(SModelAnimation());
			auto& animation = OutAnimationSet.vAnimations[OutAnimationSet.vAnimations.size() - 1];

			const auto& ai_animation = Scene->mAnimations[animation_id];
			animation.Name = ai_animation->mName.C_Str();
			animation.TicksPerSecond = static_cast<float>(ai_animation->mTicksPerSecond);
			animation.TotalTicks = static_cast<float>(ai_animation->mDuration);

			// channel = animation of a single node
			if (ai_animation->mNumChannels)
			{
				for (unsigned int channel_id = 0; channel_id < ai_animation->mNumChannels; ++channel_id)
				{
					animation.vNodeAnimation.push_back(SModelNodeAnimation());
					auto& node_animation = animation.vNodeAnimation[animation.vNodeAnimation.size() - 1];

					const auto& ai_channel = ai_animation->mChannels[channel_id];

					// Find the name-matching node
					for (const auto& node : NodeTree.vNodes)
					{
						if (node.Name.compare(ai_channel->mNodeName.C_Str()) == 0)
						{
							// We found the matching name
							node_animation.NodeID = node.ID;
							break;
						}
					}

					// Get position keys
					if (ai_channel->mNumPositionKeys)
					{
						for (unsigned int key_id = 0; key_id < ai_channel->mNumPositionKeys; ++key_id)
						{
							node_animation.vKeyPosition.push_back(SModelAnimationKeyPosition());
							auto& key = node_animation.vKeyPosition[node_animation.vKeyPosition.size() - 1];
							const auto& ai_key = ai_channel->mPositionKeys[key_id];

							key.TimeInTicks = static_cast<float>(ai_key.mTime);
							key.Key = ConvertaiVector3DToXMFLOAT3(ai_key.mValue);
						}
					}

					// Get rotation keys
					if (ai_channel->mNumRotationKeys)
					{
						for (unsigned int key_id = 0; key_id < ai_channel->mNumRotationKeys; ++key_id)
						{
							node_animation.vKeyRotation.push_back(SModelAnimationKeyRotation());
							auto& key = node_animation.vKeyRotation[node_animation.vKeyRotation.size() - 1];
							const auto& ai_key = ai_channel->mRotationKeys[key_id];

							key.TimeInTicks = static_cast<float>(ai_key.mTime);
							key.Key = ConvertaiQuaternionToXMVECTOR(ai_key.mValue);
						}
					}
					
					// Get scaling keys
					if (ai_channel->mNumScalingKeys)
					{
						for (unsigned int key_id = 0; key_id < ai_channel->mNumScalingKeys; ++key_id)
						{
							node_animation.vKeyScaling.push_back(SModelAnimationKeyScaling());
							auto& key = node_animation.vKeyScaling[node_animation.vKeyScaling.size() - 1];
							const auto& ai_key = ai_channel->mScalingKeys[key_id];

							key.TimeInTicks = static_cast<float>(ai_key.mTime);
							key.Key = ConvertaiVector3DToXMFLOAT3(ai_key.mValue);
						}
					}
				}
			}
		}
	}
}
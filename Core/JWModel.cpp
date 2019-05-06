#include "JWModel.h"
#include "JWDX.h"
#include "JWAssimpLoader.h"

using namespace JWEngine;

void JWModel::Create(JWDX& DX, STRING& BaseDirectory) noexcept
{
	// Set JWDX pointer.
	m_pDX = &DX;

	// Set BaseDirectory pointer.
	m_pBaseDirectory = &BaseDirectory;
}

void JWModel::Destroy() noexcept
{
	for (uint32_t i = 0; i < KVertexBufferCount; ++i)
	{
		JW_RELEASE(ModelVertexBuffer[i]);
	}
	JW_RELEASE(ModelIndexBuffer);
}

void JWModel::CreateMeshBuffers(const SModelData& Data, ERenderType Type) noexcept
{
	// Avoid duplicate creation
	if (m_RenderType != ERenderType::Invalid) { return; }

	if ((Type == ERenderType::Model_Static) || (Type == ERenderType::Model_Dynamic) || (Type == ERenderType::Model_Rigged))
	{
		// Set render type
		m_RenderType = Type;

		// Save the model data
		ModelData = Data;

		// Create model's vertex & index buffers
		CreateModelVertexIndexBuffers();
	}
}

void JWModel::CreateInstanceBuffer() noexcept
{
	// @important
	ModelData.VertexData.InitializeInstance();

	// Create vertex buffer #KVBIDInstancing
	m_pDX->CreateDynamicVertexBuffer(
		ModelData.VertexData.GetInstanceByteSize(),
		ModelData.VertexData.GetInstancePtrData(),
		&ModelVertexBuffer[KVBIDInstancing]);
}

auto JWModel::AddAnimationFromFile(STRING FileName) noexcept->JWModel*
{
	if (m_RenderType == ERenderType::Model_Rigged)
	{
		JWAssimpLoader loader{};
		loader.LoadAdditionalAnimationIntoRiggedModel(ModelData, *m_pBaseDirectory + KAssetDirectory, FileName);
	}

	return this;
}

auto JWModel::BakeAnimationTexture(SSizeInt TextureSize, STRING FileName) noexcept->JWModel*
{
	if (ModelData.AnimationSet.vAnimations.size())
	{
		STextureData texture_data{};
		auto& current_tex = texture_data.Texture;
		auto& current_srv = texture_data.TextureSRV;
		auto& current_tex_size = texture_data.TextureSize;

		// Set texture format we want to use
		DXGI_FORMAT texture_format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		// Describe the texture.
		D3D11_TEXTURE2D_DESC texture_descrption{};
		texture_descrption.Width = current_tex_size.Width = TextureSize.Width;
		texture_descrption.Height = current_tex_size.Height = TextureSize.Height;
		texture_descrption.MipLevels = 1;
		texture_descrption.ArraySize = 1;
		texture_descrption.Format = texture_format;
		texture_descrption.SampleDesc.Count = 1;
		texture_descrption.Usage = D3D11_USAGE_DYNAMIC;
		texture_descrption.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texture_descrption.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texture_descrption.MiscFlags = 0;

		// Create the texture.
		if (SUCCEEDED(m_pDX->GetDevice()->CreateTexture2D(&texture_descrption, nullptr, &current_tex)))
		{
			// Describe the shader resource view.
			D3D11_SHADER_RESOURCE_VIEW_DESC srv_description{};
			srv_description.Format = texture_format;
			srv_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_description.Texture2D.MostDetailedMip = 0;
			srv_description.Texture2D.MipLevels = 1;

			// Create the shader resource view.
			m_pDX->GetDevice()->CreateShaderResourceView(current_tex, &srv_description, &current_srv);

			auto& vec_animations{ ModelData.AnimationSet.vAnimations };

			uint32_t texel_count{ TextureSize.Width * TextureSize.Height };
			uint32_t texel_y_advance{ TextureSize.Width * KColorCountPerTexel };
			uint32_t data_size{ texel_count * KColorCountPerTexel };
			float* data = new float[data_size] {};

			//
			// Set animation set's info
			// (with maximum bone count = KMaxBoneCount)
			// data[0 ~ 3] = Animation ID 0 = TPose
			//
			// TPose frame count(=texel line count)
			data[0] = 1;
			// TPose texel start y index
			data[1] = 1;
			// Reserved
			data[2] = 0;
			// Reserved
			data[3] = 0;

			for (uint16_t iter_anim = 0; iter_anim < vec_animations.size(); iter_anim++)
			{
				// current animation's frame count(=texel line count)
				data[4 + iter_anim * 4] = static_cast<float>(vec_animations[iter_anim].TotalFrameCount);

				// current animation's texel start y index
				data[4 + iter_anim * 4 + 1] = data[iter_anim * 4] + data[1 + iter_anim * 4];

				// Reserved
				//data[4 + iter_anim * 4 + 2] = 0;

				// Reserved
				//data[4 + iter_anim * 4 + 3] = 0;
			}

			// Bake animations into the texture
			uint32_t start_index{ texel_y_advance };
			XMMATRIX frame_matrices[KMaxBoneCount]{};

			// TPose into FrameMatrices
			SaveTPoseIntoFrameMatrices(frame_matrices, ModelData, ModelData.NodeTree.vNodes[0], XMMatrixIdentity());

			// Bake FrameMatrices into Texture
			BakeCurrentFrameIntoTexture(start_index, frame_matrices, data);

			// Update start index
			start_index += texel_y_advance;

			for (uint16_t anim_index = 0; anim_index < vec_animations.size(); anim_index++)
			{
				// This loop iterates each animation, starting from vec_animations[0]

				float frame_time{};

				for (uint16_t frame_index = 0; frame_index < vec_animations[anim_index].TotalFrameCount; frame_index++)
				{
					frame_time = static_cast<float>(frame_index) * vec_animations[anim_index].AnimationTicksPerGameTick;

					// Current frame into FrameMatrices
					SaveAnimationFrameIntoFrameMatrices(frame_matrices, anim_index, frame_time,
						ModelData, ModelData.NodeTree.vNodes[0], XMMatrixIdentity());

					// Bake FrameMatrices into Texture
					BakeCurrentFrameIntoTexture(start_index, frame_matrices, data);

					// Update start index
					start_index += texel_y_advance;
				}
			}

			// Map data into texture
			m_pDX->UpdateDynamicResource(current_tex, data, sizeof(float)* data_size);

			// Save texture into file
			WSTRING w_fn = StringToWstring(*m_pBaseDirectory + KAssetDirectory + FileName);
			SaveDDSTextureToFile(m_pDX->GetDeviceContext(), current_tex, w_fn.c_str());

			JW_DELETE_ARRAY(data);
			JW_RELEASE(current_tex);
			JW_RELEASE(current_srv);
		}
	}

	return this;
}

PRIVATE void JWModel::SaveTPoseIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const SModelData& ModelData,
	const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept
{
	XMMATRIX accumulation = CurrentNode.Transformation * Accumulated;

	if (CurrentNode.BoneID >= 0)
	{
		auto& bone = ModelData.BoneTree.vBones[CurrentNode.BoneID];

		OutFrameMatrices[CurrentNode.BoneID] = bone.Offset * accumulation;
	}

	if (CurrentNode.vChildrenID.size())
	{
		for (auto child_id : CurrentNode.vChildrenID)
		{
			SaveTPoseIntoFrameMatrices(OutFrameMatrices, ModelData, ModelData.NodeTree.vNodes[child_id], accumulation);
		}
	}
}

PRIVATE void JWModel::SaveAnimationFrameIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const int AnimationID, const float FrameTime,
	const SModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept
{
	XMMATRIX global_transformation = CurrentNode.Transformation * Accumulated;

	if (CurrentNode.BoneID >= 0)
	{
		auto& bone = ModelData.BoneTree.vBones[CurrentNode.BoneID];
		auto& current_animation = ModelData.AnimationSet.vAnimations[AnimationID];

		for (const auto& CurrentNode_animation : current_animation.vNodeAnimation)
		{
			if (CurrentNode_animation.NodeID == CurrentNode.ID)
			{
				XMVECTOR scaling_key_a{};
				XMVECTOR rotation_key_a{};
				XMVECTOR translation_key_a{};

				// #1. Find scaling keys
				for (const auto& key : CurrentNode_animation.vKeyScaling)
				{
					if (key.TimeInTicks <= FrameTime)
					{
						scaling_key_a = XMLoadFloat3(&key.Key);
					}
				}

				// #2. Find rotation keys
				for (const auto& key : CurrentNode_animation.vKeyRotation)
				{
					if (key.TimeInTicks <= FrameTime)
					{
						rotation_key_a = key.Key;
					}
				}

				// #3. Find translation keys
				for (const auto& key : CurrentNode_animation.vKeyPosition)
				{
					if (key.TimeInTicks <= FrameTime)
					{
						translation_key_a = XMLoadFloat3(&key.Key);
					}
				}

				XMMATRIX matrix_scaling{ XMMatrixScalingFromVector(scaling_key_a) };
				XMMATRIX matrix_rotation{ XMMatrixRotationQuaternion(rotation_key_a) };
				XMMATRIX matrix_translation{ XMMatrixTranslationFromVector(translation_key_a) };

				global_transformation = matrix_scaling * matrix_rotation * matrix_translation * Accumulated;

				break;
			}
		}

		OutFrameMatrices[CurrentNode.BoneID] = bone.Offset * global_transformation;
	}

	if (CurrentNode.vChildrenID.size())
	{
		for (auto child_id : CurrentNode.vChildrenID)
		{
			SaveAnimationFrameIntoFrameMatrices(OutFrameMatrices, AnimationID, FrameTime,
				ModelData, ModelData.NodeTree.vNodes[child_id], global_transformation);
		}
	}
}

PRIVATE void JWModel::BakeCurrentFrameIntoTexture(uint32_t StartIndex, const XMMATRIX* FrameMatrices, float*& OutData) noexcept
{
	XMFLOAT4X4A current_matrix{};
	const int matrix_size_in_floats = 16;

	for (uint16_t bone_index = 0; bone_index < KMaxBoneCount; ++bone_index)
	{
		XMStoreFloat4x4(&current_matrix, FrameMatrices[bone_index]);

		memcpy(&OutData[StartIndex + bone_index * matrix_size_in_floats], current_matrix.m, sizeof(float) * matrix_size_in_floats);
	}
}

PRIVATE void JWModel::CreateModelVertexIndexBuffers() noexcept
{
	// Create vertex buffer #KVBIDModel
	if (m_RenderType == ERenderType::Model_Dynamic)
	{
		m_pDX->CreateDynamicVertexBuffer(
			ModelData.VertexData.GetVertexModelByteSize(), ModelData.VertexData.GetVertexModelPtrData(), &ModelVertexBuffer[KVBIDModel]);
	}
	else
	{
		m_pDX->CreateStaticVertexBuffer(
			ModelData.VertexData.GetVertexModelByteSize(), ModelData.VertexData.GetVertexModelPtrData(), &ModelVertexBuffer[KVBIDModel]);
	}

	if (m_RenderType == ERenderType::Model_Rigged)
	{
		// Create vertex buffer #KVBIDRigging
		m_pDX->CreateStaticVertexBuffer(
			ModelData.VertexData.GetVertexRiggingByteSize(), ModelData.VertexData.GetVertexRiggingPtrData(), &ModelVertexBuffer[KVBIDRigging]);
	}

	// Create index buffer
	m_pDX->CreateIndexBuffer(ModelData.IndexData.GetByteSize(), ModelData.IndexData.GetPtrData(), &ModelIndexBuffer);
}

auto JWModel::SetVertex(uint32_t VertexIndex, const XMVECTOR& Position, const XMFLOAT4& Color) noexcept->JWModel*
{
	if (m_RenderType == ERenderType::Model_Dynamic)
	{
		if (VertexIndex > ModelData.VertexData.GetVertexCount())
		{
			return this;
		}

		ModelData.VertexData.vVerticesModel[VertexIndex].Position = Position;
		ModelData.VertexData.vVerticesModel[VertexIndex].Diffuse = Color;
	}

	return this;
}

auto JWModel::SetVertex(uint32_t VertexIndex, const XMFLOAT3& Position, const XMFLOAT4& Color) noexcept->JWModel*
{
	if (m_RenderType == ERenderType::Model_Dynamic)
	{
		if (VertexIndex > ModelData.VertexData.GetVertexCount())
		{
			return this;
		}

		ModelData.VertexData.vVerticesModel[VertexIndex].Position = XMVectorSet(Position.x, Position.y, Position.z, 1);
		ModelData.VertexData.vVerticesModel[VertexIndex].Diffuse = Color;
	}

	return this;
}

void JWModel::UpdateModel() noexcept
{
	if (m_RenderType == ERenderType::Model_Dynamic)
	{
		m_pDX->UpdateDynamicResource(ModelVertexBuffer[KVBIDModel],
			ModelData.VertexData.GetVertexModelPtrData(),
			ModelData.VertexData.GetVertexModelByteSize());
	}
}
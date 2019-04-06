#pragma once

#include "../Core/JWAssimpLoader.h"
#include "../Core/JWDX.h"
#include "../Core/JWModel.h"
#include "../Core/JWImage.h"

namespace JWEngine
{
	class JWCamera;
	class JWEntity;

	enum EFLAGRenderOption : uint16_t
	{
		JWFlagRenderOption_UseTexture = 0b1,
		JWFlagRenderOption_UseLighting = 0b10,
		JWFlagRenderOption_UseGPUAnimation = 0b100,
		JWFlagRenderOption_UseAnimationInterpolation = 0b1000,
		JWFlagRenderOption_UseTransparency = 0b10000,
		JWFlagRenderOption_DrawNormals = 0b100000,
		JWFlagRenderOption_DrawTPose = 0b1000000,
	};
	using JWFlagRenderOption = uint16_t;

	struct SAnimationTextureData
	{
		ID3D11Texture2D*			Texture{};
		ID3D11ShaderResourceView*	TextureSRV{};
		SSizeInt					TextureSize{};
	};

	struct SAnimationState
	{
		// If no animation is set, CurrAnimationID is 0 (TPose)
		uint32_t CurrAnimationID{};

		// If SetAnimation() is called, CurrAnimationTick is reset to 0.
		float CurrAnimationTick{};

		float CurrFrameTime{};
		float NextFrameTime{};

		// Used for interpolation
		float DeltaTime{};

		bool ShouldRepeat{};
	};
	
	struct SComponentRender
	{
		// Non-owner pointer
		JWEntity*					PtrEntity{};
		uint32_t					ComponentID{};
		ERenderType					RenderType{ ERenderType::Invalid };

		// Non-owner pointer
		JWDX*						PtrDX{};
		// Non-owner pointer
		JWCamera*					PtrCamera{};
		// Non-owner pointer
		const STRING*				PtrBaseDirectory{};

		// Non-owner pointer
		JWModel*					PtrModel{};
		JWImage						Image{};

		// Non-owner pointer
		ID3D11ShaderResourceView*	PtrTexture;

		// Non-owner pointer
		SAnimationTextureData*		PtrBakedAnimationTexture;
		SAnimationState				AnimationState;

		EDepthStencilState			DepthStencilState{ EDepthStencilState::ZEnabled };
		EVertexShader				VertexShader{ EVertexShader::VSBase };
		EPixelShader				PixelShader{ EPixelShader::PSBase };

		JWFlagRenderOption			FlagRenderOption{};

		auto SetTexture(ID3D11ShaderResourceView* pShaderResourceView) noexcept
		{
			PtrTexture = pShaderResourceView;

			FlagRenderOption |= JWFlagRenderOption_UseTexture;

			return this;
		}

		auto SetVertexShader(EVertexShader Shader) noexcept
		{
			VertexShader = Shader;
			return this;
		}

		auto SetPixelShader(EPixelShader Shader) noexcept
		{
			PixelShader = Shader;
			return this;
		}

		auto SetModel(JWModel* pModel) noexcept
		{
			assert(pModel);

			if (RenderType == ERenderType::Invalid)
			{
				PtrModel = pModel;

				RenderType = PtrModel->GetRenderType();

				
				if (RenderType == ERenderType::Model_Rigged)
				{
					// @important
					VertexShader = EVertexShader::VSAnim;
				}
			}

			return this;
		}

		auto MakeImage2D(SPositionInt Position, SSizeInt Size) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Image.Create(*PtrDX, *PtrCamera);

				Image.SetPosition(XMFLOAT2(static_cast<float>(Position.X), static_cast<float>(Position.Y)));
				Image.SetSize(XMFLOAT2(static_cast<float>(Size.Width), static_cast<float>(Size.Height)));

				DepthStencilState = EDepthStencilState::ZDisabled;

				RenderType = ERenderType::Image_2D;
			}

			return this;
		}

		auto SetRenderFlag(JWFlagRenderOption Flag)
		{
			FlagRenderOption = Flag;

			return this;
		}

		auto ToggleRenderFlag(JWFlagRenderOption Flag)
		{
			FlagRenderOption ^= Flag;

			return this;
		}

		// Animation ID 0 is not an animation but TPose
		auto SetAnimation(uint32_t AnimationID, bool ShouldRepeat = true)
		{
			if (RenderType == ERenderType::Model_Rigged)
			{
				if (PtrModel->RiggedModelData.AnimationSet.vAnimations.size())
				{
					AnimationID = min(AnimationID, static_cast<uint32_t>(PtrModel->RiggedModelData.AnimationSet.vAnimations.size()));
				}
				else
				{
					// Only TPose is available
					AnimationID = 0;
				}

				// Set animation only when animation id has changed from the previous one.
				if (AnimationState.CurrAnimationID != AnimationID)
				{
					AnimationState.CurrAnimationID = AnimationID;
					AnimationState.CurrAnimationTick = 0;
					AnimationState.ShouldRepeat = ShouldRepeat;
				}
			}

			return this;
		}

		auto NextAnimation(bool ShouldRepeat = true)
		{
			if (RenderType == ERenderType::Model_Rigged)
			{
				uint32_t AnimationID = AnimationState.CurrAnimationID + 1;

				if (AnimationID > PtrModel->RiggedModelData.AnimationSet.vAnimations.size())
				{
					AnimationID = 0;
				}

				SetAnimation(AnimationID, ShouldRepeat);
			}

			return this;
		}

		auto PrevAnimation(bool ShouldRepeat = true)
		{
			if (RenderType == ERenderType::Model_Rigged)
			{
				uint32_t AnimationID = AnimationState.CurrAnimationID - 1;

				SetAnimation(AnimationID, ShouldRepeat);
			}

			return this;
		}

		// Befroe calling this function, first call CreateAnimationTexture() of ECS in order to create texture.
		auto BakeAnimationsIntoTexture(SAnimationTextureData* PtrAnimationTexture)
		{
			if (PtrModel->RiggedModelData.AnimationSet.vAnimations.size())
			{
				auto& vec_animations{ PtrModel->RiggedModelData.AnimationSet.vAnimations };

				auto& texture_size{ PtrAnimationTexture->TextureSize };
				uint32_t texel_count{ texture_size.Width * texture_size.Height };
				uint32_t texel_y_advance{ texture_size.Width * KColorCountPerTexel };
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
				SaveTPoseIntoFrameMatrices(frame_matrices, PtrModel->RiggedModelData,
					PtrModel->RiggedModelData.NodeTree.vNodes[0], XMMatrixIdentity());
				
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
							PtrModel->RiggedModelData, PtrModel->RiggedModelData.NodeTree.vNodes[0], XMMatrixIdentity());

						// Bake FrameMatrices into Texture
						BakeCurrentFrameIntoTexture(start_index, frame_matrices, data);

						// Update start index
						start_index += texel_y_advance;
					}
				}

				D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
				if (SUCCEEDED(PtrDX->GetDeviceContext()->Map(PtrAnimationTexture->Texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
				{
					memcpy(mapped_subresource.pData, data, sizeof(float) * data_size);

					PtrDX->GetDeviceContext()->Unmap(PtrAnimationTexture->Texture, 0);
				}
				
				// Save this texture data's pointer
				PtrBakedAnimationTexture = PtrAnimationTexture;

				JW_DELETE_ARRAY(data);
			}

			return this;
		}

		auto SetAnimationTexture(SAnimationTextureData* PtrAnimationTexture) noexcept
		{
			// Save this texture data's pointer
			PtrBakedAnimationTexture = PtrAnimationTexture;

			FlagRenderOption |= JWFlagRenderOption_UseGPUAnimation;

			return this;
		}

		auto SaveAnimationTextureToFile(STRING FileName) noexcept
		{
			if (PtrBakedAnimationTexture)
			{
				WSTRING w_fn = StringToWstring(*PtrBaseDirectory + KAssetDirectory + FileName);
				
				SaveDDSTextureToFile(PtrDX->GetDeviceContext(), PtrBakedAnimationTexture->Texture, w_fn.c_str());
			}

			return this;
		}

	private:
		void SaveTPoseIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const SRiggedModelData& ModelData,
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

		void SaveAnimationFrameIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const int AnimationID, const float FrameTime,
			const SRiggedModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept
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

		inline void BakeCurrentFrameIntoTexture(uint32_t StartIndex, const XMMATRIX* FrameMatrices, float*& OutData) noexcept
		{
			XMFLOAT4X4A current_matrix{};
			const int matrix_size_in_floats = 16;

			for (uint16_t bone_index = 0; bone_index < KMaxBoneCount; ++bone_index)
			{
				XMStoreFloat4x4(&current_matrix, FrameMatrices[bone_index]);
				
				memcpy(&OutData[StartIndex + bone_index * matrix_size_in_floats], current_matrix.m, sizeof(float) * matrix_size_in_floats);
			}
		}
	};
	

	class JWSystemRender
	{
	public:
		JWSystemRender() {};
		~JWSystemRender();

		void CreateSystem(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept;

		auto CreateComponent() noexcept->SComponentRender&;
		void DestroyComponent(SComponentRender& Component) noexcept;

		void Update() noexcept;

	private:
		void SetShaders(SComponentRender& Component) noexcept;

		void AnimateOnGPU(SComponentRender& Component) noexcept;
		void AnimateOnCPU(SComponentRender& Component) noexcept;

		void UpdateNodeAnimationIntoBones(bool UseInterpolation, SAnimationState& AnimationState,
			SRiggedModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void UpdateNodeTPoseIntoBones(float AnimationTime, SRiggedModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;

		void Draw(SComponentRender& Component) noexcept;
		void DrawNormals(SComponentRender& Component) noexcept;

	private:
		VECTOR<SComponentRender*>	m_vpComponents;
		JWDX*						m_pDX{};
		JWCamera*					m_pCamera{};
		STRING						m_BaseDirectory{};
		SVSCBStatic					m_VSCBStatic{};
		SVSCBRigged					m_VSCBRigged{};
		SVSCBCPUAnimation			m_VSCBCPUAnimation{};
		SVSCBGPUAnimation			m_VSCBGPUAnimation{};
	};
};
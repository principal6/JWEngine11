#pragma once

#include "../Core/JWAssimpLoader.h"
#include "../Core/JWDX.h"
#include "../Core/JWModel.h"
#include "../Core/JWImage.h"

namespace JWEngine
{
	class JWCamera;
	class JWEntity;

	enum class ERenderType : uint8_t
	{
		Invalid,

		Model_StaticModel,
		Model_RiggedModel,

		Image_2D,
	};

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
		float AnimationTime{};
		float CurrAnimationTime{};
		float NextAnimationTime{};
		float DeltaTime{};
	};

	struct SComponentRender
	{
		JWEntity*				PtrEntity{};
		uint32_t				ComponentID{};

		ERenderType				RenderType{ ERenderType::Invalid };
		JWDX*					PtrDX{};
		JWCamera*				PtrCamera{};
		const STRING*			PtrBaseDirectory{};
		JWModel					Model{};
		JWImage					Image{};
		SAnimationTextureData*	PtrBakedAnimationTexture; // Non-owner pointer
		SAnimationState			CurrentAnimationState;

		EDepthStencilState		DepthStencilState{ EDepthStencilState::ZEnabled };
		EVertexShader			VertexShader{ EVertexShader::VSBase };
		EPixelShader			PixelShader{ EPixelShader::PSBase };

		JWFlagRenderOption		FlagRenderOption{};

		auto SetTexture(ID3D11ShaderResourceView* pShaderResourceView) noexcept
		{
			JW_RELEASE(Model.TextureShaderResourceView);

			Model.TextureShaderResourceView = pShaderResourceView;

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

		auto MakeSquare(float Size, XMFLOAT2 UVMap) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeSquare(Size, UVMap);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCircle(float Radius, uint8_t Detail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCircle(Radius, Detail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCube(float Size) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCube(Size);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakePyramid(float Height, float Width) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakePyramid(Height, Width);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCone(float Height, float Radius, uint8_t Detail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCone(Height, Radius, Detail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCylinder(Height, Radius, Detail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeSphere(Radius, VerticalDetail, HorizontalDetail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCapsule(Height, Radius, VerticalDetail, HorizontalDetail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto LoadModel(ERenderType Type, STRING FileName) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				JWAssimpLoader loader;

				switch (Type)
				{
				case JWEngine::ERenderType::Model_StaticModel:
					Model.Create(*PtrDX);
					Model.SetStaticModelData(loader.LoadStaticModel(*PtrBaseDirectory + KAssetDirectory, FileName));

					RenderType = Type;
					break;
				case JWEngine::ERenderType::Model_RiggedModel:
					Model.Create(*PtrDX);
					Model.SetRiggedModelData(loader.LoadRiggedModel(*PtrBaseDirectory + KAssetDirectory, FileName));

					// @important
					VertexShader = EVertexShader::VSAnim;

					RenderType = Type;
					break;
				default:
					break;
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

		auto AddAnimation(STRING FileName)
		{
			Model.AddAnimationFromFile(*PtrBaseDirectory + KAssetDirectory, FileName);

			return this;
		}

		// Animation ID 0 is TPose
		auto SetAnimation(uint32_t AnimationID)
		{
			Model.SetAnimation(AnimationID);

			return this;
		}

		auto NextAnimation()
		{
			Model.SetNextAnimation();

			return this;
		}

		auto PrevAnimation()
		{
			Model.SetPrevAnimation();

			return this;
		}

		// To use this function, first call CreateAnimationTexture() of ECS.
		auto BakeAnimationsIntoTexture(SAnimationTextureData* PtrAnimationTexture)
		{
			if (Model.RiggedModelData.AnimationSet.vAnimations.size())
			{
				auto& vec_animations{ Model.RiggedModelData.AnimationSet.vAnimations };

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
				SaveTPoseIntoFrameMatrices(frame_matrices, Model.RiggedModelData,
					Model.RiggedModelData.NodeTree.vNodes[0], XMMatrixIdentity());
				
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
							Model.RiggedModelData, Model.RiggedModelData.NodeTree.vNodes[0], XMMatrixIdentity());

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

		auto LoadBakedAnimationFromDDS(SAnimationTextureData* PtrAnimationTexture) noexcept
		{
			// Save this texture data's pointer
			PtrBakedAnimationTexture = PtrAnimationTexture;

			FlagRenderOption |= JWFlagRenderOption_UseGPUAnimation;

			return this;
		}

		auto SaveBakedAnimationAsDDS(STRING FileName) noexcept
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

		void UpdateNodeAnimationIntoBones(bool UseInterpolation, float AnimationTime, SRiggedModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void UpdateNodeTPoseIntoBones(float AnimationTime, SRiggedModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;

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
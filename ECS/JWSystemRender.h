#pragma once

#include "../Core/JWDX.h"
#include "../Core/JWModel.h"
#include "../Core/JWImage.h"
#include "../Core/JWLineModel.h"

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
		uint32_t	CurrAnimationID{};

		// If SetAnimation() is called, CurrAnimationTick is reset to 0.
		float		CurrAnimationTick{};

		float		CurrFrameTime{};
		float		NextFrameTime{};
		float		TweeningTime{};

		uint32_t	NextAnimationID{};
	};
	
	// Every pointer is a non-owner pointer.
	struct SComponentRender
	{
		JWEntity*					PtrEntity{};
		uint32_t					ComponentID{};

		ERenderType					RenderType{ ERenderType::Invalid };
		EDepthStencilState			DepthStencilState{ EDepthStencilState::ZEnabled };
		EBlendState					BlendState{ EBlendState::Opaque };
		EVertexShader				VertexShader{ EVertexShader::VSBase };
		EPixelShader				PixelShader{ EPixelShader::PSBase };

		JWModel*					PtrModel{};
		JWImage*					PtrImage{};
		JWLineModel*				PtrLine{};

		ID3D11ShaderResourceView*	PtrTexture{};

		SAnimationTextureData*		PtrAnimationTexture{};
		SAnimationState				AnimationState{};

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

		auto SetImage2D(JWImage* pImage2D) noexcept
		{
			assert(pImage2D);

			if (RenderType == ERenderType::Invalid)
			{
				PtrImage = pImage2D;

				RenderType = ERenderType::Image_2D;
			}

			return this;
		}

		auto SetLineModel(JWLineModel* pLineModel) noexcept
		{
			assert(pLineModel);

			if (RenderType == ERenderType::Invalid)
			{
				PtrLine = pLineModel;

				RenderType = PtrLine->m_RenderType;

				if (RenderType == ERenderType::Model_Line2D)
				{
					DepthStencilState = EDepthStencilState::ZDisabled;
				}
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
		auto SetAnimation(uint32_t AnimationID, uint32_t NextAnimationID = -1)
		{
			if (RenderType == ERenderType::Model_Rigged)
			{
				if (NextAnimationID == -1)
				{
					NextAnimationID = AnimationID;
				}

				if (PtrModel->RiggedModelData.AnimationSet.vAnimations.size())
				{
					if (AnimationID == (uint32_t)-1)
					{
						AnimationID = 0;
					}

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
					AnimationState.NextAnimationID = NextAnimationID;
				}
			}

			return this;
		}

		auto NextAnimation()
		{
			SetAnimation(AnimationState.CurrAnimationID + 1);
			
			return this;
		}

		auto PrevAnimation()
		{
			SetAnimation(AnimationState.CurrAnimationID - 1);

			return this;
		}

		auto SetAnimationTexture(SAnimationTextureData* pAnimationTexture) noexcept
		{
			// Save this texture data's pointer
			PtrAnimationTexture = pAnimationTexture;

			FlagRenderOption |= JWFlagRenderOption_UseGPUAnimation;

			return this;
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

		void Execute() noexcept;

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
		SVSCBSpace					m_VSCBSpace{};
		SVSCBFlags					m_VSCBFlags{};
		SVSCBCPUAnimation			m_VSCBCPUAnimation{};
		SVSCBGPUAnimation			m_VSCBGPUAnimation{};
	};
};
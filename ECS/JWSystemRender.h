#pragma once

#include "../Core/JWDX.h"
#include "../Core/JWModel.h"
#include "../Core/JWLineModel.h"
#include "../Core/JWImage.h"
#include "../Core/JWPrimitiveMaker.h"

namespace JWEngine
{
	class JWCamera;
	//class JWModel;
	//class JWLineModel;
	//class JWImage;
	//class JWPrimitiveMaker;
	class JWEntity;
	class JWECS;

	enum class ESharedTextureType
	{
		Texture2D,
		TextureCubeMap,
	};

	enum class ESharedModelType
	{
		StaticModel,
		RiggedModel,
	};

	enum EFLAGRenderOption : uint16_t
	{
		JWFlagRenderOption_UseTexture					= 0x01,
		JWFlagRenderOption_UseLighting					= 0x02,
		JWFlagRenderOption_UseGPUAnimation				= 0x04,
		JWFlagRenderOption_UseAnimationInterpolation	= 0x08,
		JWFlagRenderOption_UseTransparency				= 0x10,
		JWFlagRenderOption_DrawTPose					= 0x20,
		JWFlagRenderOption_AlwaysSolidNoCull			= 0x40,
	};
	using JWFlagRenderOption = uint16_t;

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

		auto SetVertexShader(EVertexShader Shader) noexcept { VertexShader = Shader; return this; }
		auto SetPixelShader(EPixelShader Shader) noexcept { PixelShader = Shader; return this; }

		auto SetTexture(ID3D11ShaderResourceView* pShaderResourceView) noexcept
		{ 
			PtrTexture = pShaderResourceView;
			FlagRenderOption |= JWFlagRenderOption_UseTexture;
			return this;
		}

		auto SetModel(JWModel* pModel) noexcept
		{
			assert(pModel);

			if (RenderType == ERenderType::Invalid)
			{
				PtrModel = pModel;

				RenderType = PtrModel->GetRenderType();

				// @important!!
				VertexShader = EVertexShader::VSBase;
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

				if (RenderType == ERenderType::Model_Line2D) { DepthStencilState = EDepthStencilState::ZDisabled; }
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

		auto SetRenderFlag(JWFlagRenderOption Flag) { FlagRenderOption = Flag; return this; }
		auto ToggleRenderFlag(JWFlagRenderOption Flag) { FlagRenderOption ^= Flag; return this; }
		auto SetDepthStencilState(EDepthStencilState State) { DepthStencilState = State; return this; }

		// Animation ID 0 is not an animation but TPose
		auto SetAnimation(uint32_t AnimationID, uint32_t NextAnimationID = -1)
		{
			if (RenderType == ERenderType::Model_Rigged)
			{
				if (NextAnimationID == -1) { NextAnimationID = AnimationID; }

				if (PtrModel->ModelData.AnimationSet.vAnimations.size())
				{
					if (AnimationID == (uint32_t)-1) { AnimationID = 0; }

					AnimationID = min(AnimationID, static_cast<uint32_t>(PtrModel->ModelData.AnimationSet.vAnimations.size()));
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

		auto NextAnimation() { SetAnimation(AnimationState.CurrAnimationID + 1); return this; }
		auto PrevAnimation() { SetAnimation(AnimationState.CurrAnimationID - 1); return this; }

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
		JWSystemRender() = default;
		~JWSystemRender() = default;

		void Create(JWECS& ECS, JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept;
		void Destroy() noexcept;

		auto CreateComponent() noexcept->SComponentRender&;
		void DestroyComponent(SComponentRender& Component) noexcept;

		/// Shared resources
		void CreateSharedTexture(ESharedTextureType Type, STRING FileName) noexcept;
		void CreateSharedTextureFromSharedModel(size_t ModelIndex) noexcept;
		auto GetSharedTexture(size_t Index) noexcept->ID3D11ShaderResourceView*;

		auto CreateSharedModelPrimitive(const SModelData& ModelData) noexcept->JWModel*;
		auto CreateSharedModelDynamicPrimitive(const SModelData& ModelData) noexcept->JWModel*;
		auto CreateSharedModelFromFile(ESharedModelType Type, STRING FileName) noexcept->JWModel*;
		auto GetSharedModel(size_t Index) noexcept->JWModel*;

		auto CreateSharedLineModel() noexcept->JWLineModel*;
		auto GetSharedLineModel(size_t Index) noexcept->JWLineModel*;

		auto CreateSharedImage2D(SPositionInt Position, SSizeInt Size) noexcept->JWImage*;
		auto GetSharedImage2D(size_t Index) noexcept->JWImage*;

		void CreateAnimationTextureFromFile(STRING FileName) noexcept;
		auto GetAnimationTexture(size_t Index) noexcept->SAnimationTextureData*;

		/// Bounidng volume
		void AddBoundingVolumeInstance(float Radius, const XMFLOAT3& Translation) noexcept;
		void UpdateBoundingVolume() noexcept;

		void Execute() noexcept;

		void SetUniversalRasterizerState(ERasterizerState State) noexcept;

		void ToggleWireFrame() noexcept;
		void ToggleNormalDrawing() noexcept;
		void ToggleLighting() noexcept;
		void ToggleBoundingVolumeDrawing() noexcept;

		/// Object getter
		auto& BoundingVolume() noexcept { return m_BoundingVolume; };
		auto& PrimitiveMaker() noexcept { return m_PrimitiveMaker; }

	private:
		void SetShaders(SComponentRender& Component) noexcept;

		void AnimateOnGPU(SComponentRender& Component) noexcept;
		void AnimateOnCPU(SComponentRender& Component) noexcept;

		void UpdateNodeAnimationIntoBones(bool UseInterpolation, SAnimationState& AnimationState,
			SModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void UpdateNodeTPoseIntoBones(float AnimationTime, SModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;

		void Draw(SComponentRender& Component) noexcept;
		void DrawNormals(SComponentRender& Component) noexcept;
		void DrawInstancedBoundingVolume() noexcept;
		__declspec(deprecated("This function is deprecated. Use DrawInstancedBoundingVolume() instead."))
			void DrawBoundingVolumesNoInstancing(SComponentRender& Component) noexcept;
		
	private:
		VECTOR<SComponentRender*>	m_vpComponents;

		JWECS*						m_pECS{};
		JWDX*						m_pDX{};
		JWCamera*					m_pCamera{};
		STRING						m_BaseDirectory{};

		SVSCBSpace					m_VSCBSpace{};
		SVSCBFlags					m_VSCBFlags{};
		SVSCBCPUAnimationData		m_VSCBCPUAnimation{};
		SVSCBGPUAnimationData		m_VSCBGPUAnimation{};

		/// Shared resources(texture, model data, animation texture)
		VECTOR<ID3D11ShaderResourceView*>	m_vpSharedSRV;
		VECTOR<SAnimationTextureData>		m_vAnimationTextureData;
		VECTOR<JWModel>						m_vSharedModel;
		VECTOR<JWLineModel>					m_vSharedLineModel;
		VECTOR<JWImage>						m_vSharedImage2D;

		// Primitive maker (for shared resources)
		JWPrimitiveMaker					m_PrimitiveMaker{};

		// Bounding volume
		JWModel						m_BoundingVolume{};

		ERasterizerState			m_UniversalRasterizerState{ ERasterizerState::SolidNoCull };
		ERasterizerState			m_OldUniversalRasterizerState{ ERasterizerState::SolidNoCull };
		bool						m_ShouldDrawNormals{ false };
		bool						m_ShouldLight{ true };
		bool						m_ShouldDrawBoundingVolumes{ false };
	};
};
#pragma once

#include "../Core/JWDX.h"
#include "../Core/JWModel.h"
#include "../Core/JWLineModel.h"
#include "../Core/JWImage.h"
#include "../Core/JWPrimitiveMaker.h"
#include "../Core/JWTerrainGenerator.h"

namespace JWEngine
{
	//class JWModel;
	//class JWLineModel;
	//class JWImage;
	//class JWPrimitiveMaker;
	class JWEntity;
	class JWECS;

	enum class ETextureType
	{
		Diffuse,
		Normal,
	};

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

	enum EFLAGComponentRenderOption : uint32_t
	{
		JWFlagComponentRenderOption_UseDiffuseTexture			= 0x0001,
		JWFlagComponentRenderOption_UseNormalTexture			= 0x0002,
		JWFlagComponentRenderOption_UseGPUAnimation				= 0x0004,
		JWFlagComponentRenderOption_UseAnimationInterpolation	= 0x0008,
		JWFlagComponentRenderOption_UseTransparency				= 0x0010,
		JWFlagComponentRenderOption_GetLit						= 0x0020,
		JWFlagComponentRenderOption_DrawTPose					= 0x0040,
		JWFlagComponentRenderOption_AlwaysSolidNoCull			= 0x0080,
	};
	using JWFlagComponentRenderOption = uint32_t;

	enum EFLAGSystemRenderOption : uint16_t
	{
		JWFlagSystemRenderOption_DrawNormals			= 0x01,
		JWFlagSystemRenderOption_DrawBoundingVolumes	= 0x02,
		JWFlagSystemRenderOption_DrawCameras			= 0x04,
		JWFlagSystemRenderOption_DrawViewFrustum		= 0x08,
		JWFlagSystemRenderOption_UseLighting			= 0x10,
		JWFlagSystemRenderOption_UseFrustumCulling		= 0x20,
	};
	using JWFlagSystemRenderOption = uint16_t;

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
		STerrainData*				PtrTerrain{};

		// Texture used in PS
		ID3D11ShaderResourceView*	PtrTextureDiffuse{};
		ID3D11ShaderResourceView*	PtrTextureNormal{};

		// Animation texture is used in VS, not PS
		STextureData*				PtrAnimationTexture{};
		SAnimationState				AnimationState{};

		JWFlagComponentRenderOption	FlagComponentRenderOption{};

		auto SetVertexShader(EVertexShader Shader) noexcept { VertexShader = Shader; return this; }
		auto SetPixelShader(EPixelShader Shader) noexcept { PixelShader = Shader; return this; }

		auto SetTexture(ETextureType Type, ID3D11ShaderResourceView* pShaderResourceView) noexcept
		{ 
			switch (Type)
			{
			case JWEngine::ETextureType::Diffuse:
				PtrTextureDiffuse = pShaderResourceView;
				FlagComponentRenderOption |= JWFlagComponentRenderOption_UseDiffuseTexture;
				break;
			case JWEngine::ETextureType::Normal:
				PtrTextureNormal = pShaderResourceView;
				FlagComponentRenderOption |= JWFlagComponentRenderOption_UseNormalTexture;
				break;
			default:
				break;
			}
			
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

		auto SetTerrain(STerrainData* pTerrain) noexcept
		{
			assert(pTerrain);

			if (RenderType == ERenderType::Invalid)
			{
				PtrTerrain = pTerrain;

				RenderType = ERenderType::Terrain;
			}

			return this;
		}

		auto AddRenderFlag(JWFlagComponentRenderOption Flag) { FlagComponentRenderOption |= Flag; return this; }
		auto SetRenderFlag(JWFlagComponentRenderOption Flag) { FlagComponentRenderOption = Flag; return this; }
		auto ToggleRenderFlag(JWFlagComponentRenderOption Flag) { FlagComponentRenderOption ^= Flag; return this; }
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

		auto SetAnimationTexture(STextureData* pAnimationTexture) noexcept
		{
			// Save this texture data's pointer
			PtrAnimationTexture = pAnimationTexture;
			FlagComponentRenderOption |= JWFlagComponentRenderOption_UseGPUAnimation;
			return this;
		}
	};
	

	class JWSystemRender
	{
	public:
		JWSystemRender() = default;
		~JWSystemRender() = default;

		void Create(JWECS& ECS, JWDX& DX, STRING BaseDirectory) noexcept;
		void Destroy() noexcept;

		auto CreateComponent(JWEntity* pEntity) noexcept->SComponentRender&;
		void DestroyComponent(SComponentRender& Component) noexcept;

		// Shared resources
		void CreateSharedTexture(ESharedTextureType Type, STRING FileName) noexcept;
		void CreateSharedTextureFromSharedModel(size_t ModelIndex) noexcept;
		auto GetSharedTexture(size_t Index) noexcept->ID3D11ShaderResourceView*;

		auto CreateSharedModelFromModelData(const SModelData& ModelData) noexcept->JWModel*;
		auto CreateDynamicSharedModelFromModelData(const SModelData& ModelData) noexcept->JWModel*;
		auto CreateSharedModelFromFile(ESharedModelType Type, STRING FileName) noexcept->JWModel*;
		auto GetSharedModel(size_t Index) noexcept->JWModel*;

		auto CreateSharedLineModel() noexcept->JWLineModel*;
		auto GetSharedLineModel(size_t Index) noexcept->JWLineModel*;

		auto CreateSharedImage2D(SPositionInt Position, SSizeInt Size) noexcept->JWImage*;
		auto GetSharedImage2D(size_t Index) noexcept->JWImage*;

		auto CreateSharedTerrainFromFile(const STRING& FileName, float HeightFactor) noexcept->STerrainData*;
		auto GetSharedTerrain(size_t Index) noexcept->STerrainData*;

		void CreateAnimationTextureFromFile(STRING FileName) noexcept;
		auto GetAnimationTexture(size_t Index) noexcept->STextureData*;
		
		// Bounidng volume
		void AddBoundingVolumeInstance(float Radius, const XMVECTOR& Translation) noexcept;
		void EraseBoundingVolumeInstance(uint32_t InstanceID) noexcept;
		void UpdateBoundingVolumeInstance(uint32_t InstanceID, float Radius, const XMVECTOR& Translation) noexcept;

		void Execute() noexcept;

		void SetUniversalRasterizerState(ERasterizerState State) noexcept;

		void SetSystemRenderFlag(JWFlagSystemRenderOption Flag) noexcept;
		void ToggleSystemRenderFlag(JWFlagSystemRenderOption Flag) noexcept;
		void ToggleWireFrame() noexcept;

		auto GetFrustumCulledEntityCount() const noexcept { return m_FrustumCulledEntityCount; }

		// Object getter
		auto& BoundingVolume() noexcept { return m_BoundingVolume; }
		auto& PrimitiveMaker() noexcept { return m_PrimitiveMaker; }

	private:
		void SetShaders(SComponentRender& Component) noexcept;

		void AnimateOnGPU(SComponentRender& Component) noexcept;
		void AnimateOnCPU(SComponentRender& Component) noexcept;

		void UpdateNodeAnimationIntoBones(bool UseInterpolation, SAnimationState& AnimationState,
			SModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void UpdateNodeTPoseIntoBones(float AnimationTime, SModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;

		// Bounding volume
		inline void UpdateBoundingVolumes() noexcept;

		// Frustum culling
		auto IsCulledByViewFrustum(const SComponentRender* pRender) const noexcept->bool;

		void Draw(SComponentRender& Component) noexcept;
		void DrawNormals(SComponentRender& Component) noexcept;
		void DrawInstancedBoundingVolume() noexcept;
		__declspec(deprecated("This function is deprecated. Use DrawInstancedBoundingVolume() instead."))
			void DrawNonInstancedBoundingVolumes(SComponentRender& Component) noexcept;
		
	private:
		VECTOR<SComponentRender*>	m_vpComponents;

		JWECS*						m_pECS{};
		JWDX*						m_pDX{};
		STRING						m_BaseDirectory{};

		SVSCBSpace					m_VSCBSpace{};
		SVSCBFlags					m_VSCBFlags{};
		SVSCBCPUAnimationData		m_VSCBCPUAnimation{};
		SVSCBGPUAnimationData		m_VSCBGPUAnimation{};
		SPSCBFlags					m_PSCBFlags{};

		// Shared resources(texture, model data, animation texture)
		VECTOR<STextureData>				m_vSharedTextureData;
		VECTOR<STextureData>				m_vAnimationTextureData;
		VECTOR<JWModel>						m_vSharedModel;
		VECTOR<JWLineModel>					m_vSharedLineModel;
		VECTOR<JWImage>						m_vSharedImage2D;
		VECTOR<STerrainData>				m_vSharedTerrain;

		// Primitive maker (for shared resources)
		JWPrimitiveMaker					m_PrimitiveMaker{};

		// Bounding volume
		JWModel						m_BoundingVolume{};

		ERasterizerState			m_UniversalRasterizerState{ ERasterizerState::SolidNoCull };
		ERasterizerState			m_OldUniversalRasterizerState{ ERasterizerState::SolidNoCull };

		JWFlagSystemRenderOption	m_FlagSystemRenderOption{};
		
		mutable uint32_t			m_FrustumCulledEntityCount{};

		// Terrain
		JWTerrainGenerator			m_TerrainGenerator{};
	};
};
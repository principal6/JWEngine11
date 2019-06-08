#pragma once

#include "../Core/JWDX.h"
#include "../Core/JWModel.h"
#include "../Core/JWLineModel.h"
#include "../Core/JWImage.h"
#include "../Core/JWPrimitiveMaker.h"
#include "../Core/JWTerrainGenerator.h"

namespace JWEngine
{
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
		JWFlagComponentRenderOption_NeverDrawNormals			= 0x0100,
	};
	using JWFlagComponentRenderOption = uint32_t;

	enum EFLAGSystemRenderOption : uint16_t
	{
		JWFlagSystemRenderOption_DrawNormals			= 0x01,
		JWFlagSystemRenderOption_DrawBoundingSpheres	= 0x02,
		JWFlagSystemRenderOption_DrawSubBoundingSpheres = 0x04,
		JWFlagSystemRenderOption_DrawCameras			= 0x08,
		JWFlagSystemRenderOption_DrawViewFrustum		= 0x10,
		JWFlagSystemRenderOption_UseLighting			= 0x20,
		JWFlagSystemRenderOption_UseFrustumCulling		= 0x40,
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
	
	// All member pointers are non-owners.
	struct SComponentRender
	{
		SComponentRender(EntityIndexType _EntityIndex, ComponentIndexType _ComponentIndex) :
			EntityIndex{ _EntityIndex }, ComponentIndex{ _ComponentIndex } {};

		EntityIndexType				EntityIndex{};
		ComponentIndexType			ComponentIndex{};

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
		friend class JWEntity;

	public:
		JWSystemRender() = default;
		~JWSystemRender() = default;

		void Create(JWECS& ECS, JWDX& DX, const SSize2& WindowSize, STRING BaseDirectory) noexcept;
		void Destroy() noexcept;

		// ### Shared Resources ###
		// Shared Texture
		void CreateSharedTexture(ESharedTextureType Type, STRING FileName) noexcept;
		void CreateSharedTextureFromSharedModel(size_t ModelIndex) noexcept;
		auto GetSharedTexture(size_t Index) noexcept->ID3D11ShaderResourceView*;
		// Shared Model
		auto CreateSharedModelFromModelData(const SModelData& ModelData) noexcept->JWModel*;
		auto CreateDynamicSharedModelFromModelData(const SModelData& ModelData) noexcept->JWModel*;
		auto CreateSharedModelFromFile(ESharedModelType Type, const STRING& FileName, const WSTRING& OverrideTextureFN = L"") noexcept->JWModel*;
		auto GetSharedModel(size_t Index) noexcept->JWModel*;
		// Shared LineModel
		auto CreateSharedLineModel() noexcept->JWLineModel*;
		auto GetSharedLineModel(size_t Index) noexcept->JWLineModel*;
		// Shared Image2D
		auto CreateSharedImage2D(SPosition2 Position, SSize2 Size) noexcept->JWImage*;
		auto GetSharedImage2D(size_t Index) noexcept->JWImage*;
		// Shared Terrain
		auto CreateSharedTerrainFromHeightMap(const STRING& HeightMapFN, float HeightFactor, float XYSizeFactor) noexcept->STerrainData*;
		auto CreateSharedTerrainFromTRN(const STRING& FileName) noexcept->STerrainData*;
		auto GetSharedTerrain(size_t Index) noexcept->STerrainData*;
		// AnimationTexture
		void CreateAnimationTextureFromFile(STRING FileName) noexcept;
		auto GetAnimationTexture(size_t Index) noexcept->STextureData*;
		
		/// ### Bounidng ellipsoid ###
		///void AddBoundingEllipsoidInstance(const XMMATRIX& WorldMatrix) noexcept;
		///void EraseBoundingEllipsoidInstance(uint32_t InstanceID) noexcept;
		///void UpdateBoundingEllipsoidInstance(uint32_t InstanceID, const XMMATRIX& WorldMatrix) noexcept;

		// ### Bounidng sphere ###
		void AddBoundingSphereInstance(const XMMATRIX& WorldMatrix) noexcept;
		void EraseBoundingSphereInstance(uint32_t InstanceID) noexcept;
		void UpdateBoundingSphereInstance(uint32_t InstanceID, const XMMATRIX& WorldMatrix) noexcept;

		void Execute() noexcept;

		// @important:
		// This function must be called when DisplayMode has been changed.
		void UpdateImage2Ds() noexcept;

		void SetUniversalRasterizerState(ERasterizerState State) noexcept;
		void SetSystemRenderFlag(JWFlagSystemRenderOption Flag) noexcept;
		void ToggleSystemRenderFlag(JWFlagSystemRenderOption Flag) noexcept;
		void ToggleWireFrame() noexcept;

		// Frustum culling
		auto GetFrustumCulledEntityCount() const noexcept { return m_FrustumCulledEntityCount; }
		auto GetFrustumCulledTerrainNodeCount() const noexcept { return m_FrustumCulledTerrainNodeCount; }

		// Object getter
		///auto& BoundingEllipsoid() noexcept { return m_BoundingEllipsoid; }
		auto& BoundingSphereModel() noexcept { return m_BoundingSphereModel; }
		auto& PrimitiveMaker() noexcept { return m_PrimitiveMaker; }
		auto& TerrainGenerator() noexcept { return m_TerrainGenerator; }

	// Only accesible for JWEntity
	private:
		auto CreateComponent(EntityIndexType EntityIndex) noexcept->ComponentIndexType;
		void DestroyComponent(ComponentIndexType ComponentIndex) noexcept;
		auto GetComponentPtr(ComponentIndexType ComponentIndex) noexcept->SComponentRender*;

	private:
		void SetShaders(SComponentRender& Component) noexcept;

		void AnimateOnGPU(SComponentRender& Component) noexcept;
		void AnimateOnCPU(SComponentRender& Component) noexcept;

		void UpdateNodeAnimationIntoBones(bool UseInterpolation, SAnimationState& AnimationState,
			SModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void UpdateNodeTPoseIntoBones(float AnimationTime, SModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;

		/// Bounding ellipsoid
		///inline void UpdateBoundingEllipsoidInstanceBuffer() noexcept;

		// Bounding sphere
		inline void UpdateBoundingSphereInstanceBuffer() noexcept;

		/// Frustum culling with bounding ellipsoid
		///auto IsUnitSphereCulledByViewFrustum(const XMMATRIX& EllipsoidWorld) const noexcept->bool;

		// Frustum culling with bounding sphere
		auto IsSphereCulledByViewFrustum(float Radius, const XMVECTOR& Center) const noexcept->bool;

		void ExecuteComponent(SComponentRender& Component) noexcept;

		void Draw(SComponentRender& Component) noexcept;
		void DrawNormals(SComponentRender& Component) noexcept;

		void DrawInstancedBoundingSpheres() noexcept;
		void DrawNonInstancedBoundingSpheres(float Radius, const XMVECTOR& Center) noexcept;

		///void DrawInstancedBoundingEllipsoids() noexcept;
		///void DrawNonInstancedBoundingEllipsoids(const XMMATRIX& EllipsoidWorld) noexcept;
		
	private:
		VECTOR<SComponentRender>	m_vComponents;

		JWECS*						m_pECS{};
		JWDX*						m_pDX{};
		const SSize2*				m_pWindowSize{};
		STRING						m_BaseDirectory{};

		SVSCBSpace					m_VSCBSpace{};
		SVSCBFlags					m_VSCBFlags{};
		SVSCBCPUAnimationData		m_VSCBCPUAnimation{};
		SVSCBGPUAnimationData		m_VSCBGPUAnimation{};
		SPSCBFlags					m_PSCBFlags{};

		// Shared resources(texture, model data, animation texture)
		VECTOR<STextureData>		m_vSharedTextureData;
		VECTOR<STextureData>		m_vAnimationTextureData;
		VECTOR<JWModel>				m_vSharedModel;
		VECTOR<JWLineModel>			m_vSharedLineModel;
		VECTOR<JWImage>				m_vSharedImage2D;
		VECTOR<STerrainData>		m_vSharedTerrain;

		// Primitive maker (for shared resources)
		JWPrimitiveMaker			m_PrimitiveMaker{};

		/// Bounding ellipsoid
		///JWModel						m_BoundingEllipsoid{};

		// Bounding sphere
		JWModel						m_BoundingSphereModel{};

		ERasterizerState			m_UniversalRasterizerState{ ERasterizerState::SolidNoCull };
		ERasterizerState			m_OldUniversalRasterizerState{ ERasterizerState::SolidNoCull };

		JWFlagSystemRenderOption	m_FlagSystemRenderOption{};
		
		mutable uint32_t			m_FrustumCulledEntityCount{};
		mutable uint32_t			m_FrustumCulledTerrainNodeCount{};

		// Terrain
		JWTerrainGenerator			m_TerrainGenerator{};
	};
};
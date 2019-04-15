#pragma once

#include "../Core/JWAssimpLoader.h"
#include "../Core/JWPrimitiveMaker.h"
#include "JWEntity.h"

namespace JWEngine
{
	class JWDX;
	class JWCamera;

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

	class JWECS final
	{
	public:
		JWECS() = default;
		~JWECS() = default;

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept;
		void Destroy() noexcept;
		
		// -------------------------
		// --- Entity management ---
		auto CreateEntity(STRING EntityName) noexcept->JWEntity*;
		auto GetEntity(uint32_t index) noexcept->JWEntity*;
		auto GetEntityByName(STRING EntityName) noexcept->JWEntity*;
		auto GetUniqueEntity(EEntityType Type) noexcept->JWEntity*;
		// Called by JWEntity::SetEntityType()
		void SetUniqueEntity(JWEntity* PtrEntity, EEntityType Type) noexcept;
		void DestroyEntity(uint32_t index) noexcept;

		// Picking
		void PickEntityTriangle(XMVECTOR& RayOrigin, XMVECTOR& RayDirection) noexcept;
		// PositionIndex = { 0, 1, 2 }
		auto GetPickedTrianglePosition(uint32_t PositionIndex) const noexcept->const XMVECTOR&;

		// ------------------------
		// --- Shared resources ---
		void CreateSharedTexture(ESharedTextureType Type, STRING FileName) noexcept;
		void CreateSharedTextureFromSharedModel(size_t ModelIndex) noexcept;
		auto GetSharedTexture(size_t Index) noexcept->ID3D11ShaderResourceView*;

		auto CreateSharedModelPrimitive(const SNonRiggedModelData& ModelData) noexcept->JWModel*;
		auto CreateSharedModelDynamicPrimitive(const SNonRiggedModelData& ModelData) noexcept->JWModel*;
		auto CreateSharedModelFromFile(ESharedModelType Type, STRING FileName) noexcept->JWModel*;
		auto GetSharedModel(size_t Index) noexcept->JWModel*;

		auto CreateSharedLineModel() noexcept->JWLineModel*;
		auto GetSharedLineModel(size_t Index) noexcept->JWLineModel*;

		auto CreateSharedImage2D(SPositionInt Position, SSizeInt Size) noexcept->JWImage*;
		auto GetSharedImage2D(size_t Index) noexcept->JWImage*;

		void CreateAnimationTextureFromFile(STRING FileName) noexcept;
		auto GetAnimationTexture(size_t Index) noexcept->SAnimationTextureData*;

		// ---------------
		// --- Execute ---
		void ExecuteSystems() noexcept;

		// ---------------
		// --- Getters ---
		auto& SystemTransform() noexcept { return m_SystemTransform; }
		auto& SystemRender() noexcept { return m_SystemRender; }
		auto& SystemLight() noexcept { return m_SystemLight; }
		auto& PrimitiveMaker() noexcept { return m_PrimitiveMaker; }

	private:
		// Returns t value
		__forceinline auto PickTriangle(XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2,
			XMVECTOR& RayOrigin, XMVECTOR& RayDirection, XMVECTOR& t_cmp) noexcept->XMVECTOR;
		__forceinline auto IsPointInTriangle(XMVECTOR& Point, XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2) noexcept->bool;

	private:
		JWDX*					m_pDX{};
		STRING					m_BaseDirectory{};

		JWSystemTransform		m_SystemTransform{};
		JWSystemRender			m_SystemRender{};
		JWSystemLight			m_SystemLight{};

		VECTOR<JWEntity*>		m_vpEntities;
		MAP<STRING, uint64_t>	m_mapEntityNames;
		JWEntity*				m_pUniqueEntities[KUniquePredefinedEntityCount]{};

		// Shared resources(texture, model data, animation texture)
		VECTOR<ID3D11ShaderResourceView*>	m_vpSharedSRV;
		VECTOR<SAnimationTextureData>		m_vAnimationTextureData;
		VECTOR<JWModel>						m_vSharedModel;
		VECTOR<JWLineModel>					m_vSharedLineModel;
		VECTOR<JWImage>						m_vSharedImage2D;

		JWPrimitiveMaker					m_PrimitiveMaker{};

		XMVECTOR							m_PickedTriangle[3]{};
	};
};


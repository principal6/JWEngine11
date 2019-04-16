#pragma once

#include "../Core/JWAssimpLoader.h"
#include "../Core/JWPrimitiveMaker.h"
#include "JWEntity.h"

namespace JWEngine
{
	class JWDX;
	class JWWin32Window;
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
		void Create(JWDX& DX, JWCamera& Camera, JWWin32Window& Window, STRING BaseDirectory) noexcept;
		void Destroy() noexcept;
		
		// -------------------------
		// --- Entity management ---
		
		// Creates user-defined entity
		auto CreateEntity(STRING EntityName) noexcept->JWEntity*;

		// Creates unique entity
		auto CreateEntity(EEntityType Type) noexcept->JWEntity*;

		auto GetEntity(uint32_t index) noexcept->JWEntity*;
		auto GetEntityByName(STRING EntityName) noexcept->JWEntity*;
		auto GetEntityByType(EEntityType Type) noexcept->JWEntity*;
		void DestroyEntity(uint32_t index) noexcept;

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
		auto& SystemPhysics() noexcept { return m_SystemPhysics; }
		auto& PrimitiveMaker() noexcept { return m_PrimitiveMaker; }

	private:
		JWDX*					m_pDX{};
		JWWin32Window*			m_pWindow{};
		STRING					m_BaseDirectory{};

		JWSystemTransform		m_SystemTransform{};
		JWSystemRender			m_SystemRender{};
		JWSystemLight			m_SystemLight{};
		JWSystemPhysics			m_SystemPhysics{};

		VECTOR<JWEntity*>		m_vpEntities;
		MAP<STRING, uint64_t>	m_mapEntityNames;

		// Shared resources(texture, model data, animation texture)
		VECTOR<ID3D11ShaderResourceView*>	m_vpSharedSRV;
		VECTOR<SAnimationTextureData>		m_vAnimationTextureData;
		VECTOR<JWModel>						m_vSharedModel;
		VECTOR<JWLineModel>					m_vSharedLineModel;
		VECTOR<JWImage>						m_vSharedImage2D;

		JWPrimitiveMaker					m_PrimitiveMaker{};
	};
};


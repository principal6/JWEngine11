#pragma once

#include "../Core/JWAssimpLoader.h"
#include "../Core/JWPrimitiveMaker.h"
#include "JWEntity.h"

namespace JWEngine
{
	class JWDX;
	class JWCamera;
	
	enum class ESharedResourceType
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
		~JWECS();

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept;

		auto CreateEntity() noexcept->JWEntity&;
		auto GetEntity(uint32_t index) noexcept->JWEntity*;
		void DestroyEntity(uint32_t index) noexcept;

		void CreateSharedResource(ESharedResourceType Type, STRING FileName) noexcept;
		void CreateSharedResourceFromSharedModel(size_t ModelIndex) noexcept;
		auto GetSharedResource(size_t Index) noexcept->ID3D11ShaderResourceView*;

		auto CreateSharedModelSquare(float Size, XMFLOAT2 UVMap) noexcept->JWModel*;
		auto CreateSharedModelCircle(float Radius, uint8_t Detail) noexcept->JWModel*;
		auto CreateSharedModelCube(float Size) noexcept->JWModel*;
		auto CreateSharedModelPyramid(float Height, float Width) noexcept->JWModel*;
		auto CreateSharedModelCone(float Height, float Radius, uint8_t Detail) noexcept->JWModel*;
		auto CreateSharedModelCylinder(float Height, float Radius, uint8_t Detail) noexcept->JWModel*;
		auto CreateSharedModelSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->JWModel*;
		auto CreateSharedModelCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->JWModel*;
		auto CreateSharedModelFromFile(ESharedModelType Type, STRING FileName) noexcept->JWECS&;
		auto GetSharedModel(size_t Index) noexcept->JWModel*;

		auto CreateSharedImage2D(SPositionInt Position, SSizeInt Size) noexcept->JWImage*;
		auto GetSharedImage2D(size_t Index) noexcept->JWImage*;

		auto AddAnimationToModelFromFile(size_t Index, STRING FileName) noexcept->JWECS&;

		void CreateAnimationTexture(SSizeInt TextureSize) noexcept;
		void CreateAnimationTextureFromFile(STRING FileName) noexcept;
		auto GetAnimationTexture(size_t Index) noexcept->SAnimationTextureData*;

		void ExecuteSystems() noexcept;

		auto& SystemTransform() noexcept { return m_SystemTransform; }
		auto& SystemRender() noexcept { return m_SystemRender; }
		auto& SystemLight() noexcept { return m_SystemLight; }

	private:
		JWDX*				m_pDX{};
		STRING				m_BaseDirectory{};

		JWSystemTransform	m_SystemTransform{};
		JWSystemRender		m_SystemRender{};
		JWSystemLight		m_SystemLight{};

		VECTOR<JWEntity*>	m_vpEntities;

		// Shared resources(texture, model data, animation texture)
		VECTOR<ID3D11ShaderResourceView*>	m_vpSharedSRV;
		VECTOR<SAnimationTextureData>		m_vAnimationTextureData;
		VECTOR<JWModel>						m_vSharedModel;
		VECTOR<JWImage>						m_vSharedImage2D;
	};
};


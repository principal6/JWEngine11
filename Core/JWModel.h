#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWDX;

	class JWModel
	{
	public:
		JWModel() = default;
		~JWModel() = default;

		void Create(JWDX& DX, STRING& BaseDirectory) noexcept;
		void Destroy() noexcept;

		void SetNonRiggedModelData(const SNonRiggedModelData& ModelData) noexcept;
		void SetDynamicModelData(const SNonRiggedModelData& ModelData) noexcept;
		void SetRiggedModelData(const SRiggedModelData& ModelData) noexcept;

		// ---------------------------------
		// --- Animation related methods ---
		auto AddAnimationFromFile(STRING FileName) noexcept->JWModel*;
		auto BakeAnimationTexture(SSizeInt TextureSize, STRING FileName) noexcept->JWModel*;

		// Only available when it's dynamic model
		auto SetVertex(uint32_t VertexIndex, XMFLOAT3 Position, XMFLOAT4 Color) noexcept->JWModel*;

		// Only available when it's dynamic model
		void UpdateModel() noexcept;

		auto GetRenderType() const noexcept { return m_RenderType; };
		auto GetTextureFileName() const noexcept->WSTRING
		{
			if (m_RenderType == ERenderType::Model_Rigged)
			{ return RiggedModelData.TextureFileNameW; }
			else 
			{ return NonRiggedModelData.TextureFileNameW; }
		}

	public:
		ID3D11Buffer*		ModelVertexBuffer{};
		ID3D11Buffer*		ModelIndexBuffer{};
		SNonRiggedModelData	NonRiggedModelData{};
		SRiggedModelData	RiggedModelData{};

		ID3D11Buffer*		NormalVertexBuffer{};
		ID3D11Buffer*		NormalIndexBuffer{};
		SLineModelData		NormalData{};

	private:
		void CreateModelVertexIndexBuffers() noexcept;

		// ---------------------------------
		// --- Animation related methods ---
		void SaveTPoseIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const SRiggedModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void SaveAnimationFrameIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const int AnimationID, const float FrameTime,
			const SRiggedModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void BakeCurrentFrameIntoTexture(uint32_t StartIndex, const XMMATRIX* FrameMatrices, float*& OutData) noexcept;

	private:
		JWDX*			m_pDX{};
		const STRING*	m_pBaseDirectory{};
		ERenderType		m_RenderType{ ERenderType::Invalid };
	};
};

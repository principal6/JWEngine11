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

		void Create(JWDX& DX, const STRING& BaseDirectory, const STRING& ModelName) noexcept;
		void Destroy() noexcept;

		void CreateMeshBuffers(const SModelData& Data, ERenderType Type) noexcept;
		void CreateInstanceBuffer() noexcept;

		// ---------------------------------
		// --- Animation related methods ---
		auto AddAnimationFromFile(const STRING& FileName) noexcept->JWModel*;

		// Before calling this function,
		// first you must add all the animations you want to bake into texture
		// by calling AddAnimationFromFile()
		auto BakeAnimationTexture(SSize2 TextureSize, const STRING& FileName) noexcept->JWModel*;

		// Only available when it's dynamic model
		// @important
		// You must call UpdateModel() after finishing all SetVertex() calls.
		auto SetVertex(uint32_t VertexIndex, const XMVECTOR& Position, const XMFLOAT4& Color) noexcept->JWModel*;

		// Only available when it's dynamic model
		// @important
		// You must call UpdateModel() after finishing all SetVertex() calls.
		auto SetVertex(uint32_t VertexIndex, const XMFLOAT3& Position, const XMFLOAT4& Color) noexcept->JWModel*;

		// Only available when it's dynamic model
		// @important: Must be called after SetVertex()
		void UpdateModel() noexcept;

		auto GetRenderType() const noexcept { return m_RenderType; };
		auto GetTextureFileName() const noexcept->const WSTRING& { return ModelData.TextureFileNameW; }
		auto GetModelName() const noexcept->const STRING& { return m_ModelName; }

	public:
		ID3D11Buffer*			ModelVertexBuffer[KVertexBufferCount]{};
		ID3D11Buffer*			ModelIndexBuffer{};
		SModelData				ModelData{};

		// For collision mesh
		VECTOR<XMVECTOR>				vPositionVertex{};
		VECTOR<size_t>					vPositionVertexIndexFromVertexIndex{};
		VECTOR<VECTOR<size_t>>			vFaceWithPositionVertex{};

	private:
		void CreateModelVertexIndexBuffers() noexcept;

		// ---------------------------------
		// --- Animation related methods ---
		void SaveTPoseIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const SModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void SaveAnimationFrameIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const int AnimationID, const float FrameTime,
			const SModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void BakeCurrentFrameIntoTexture(uint32_t StartIndex, const XMMATRIX* FrameMatrices, float*& OutData) noexcept;

	private:
		JWDX*			m_pDX{};
		const STRING*	m_pBaseDirectory{};
		ERenderType		m_RenderType{ ERenderType::Invalid };
		STRING			m_ModelName{};
	};
};

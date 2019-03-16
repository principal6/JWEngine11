#pragma once

#include "../Core/JWWin32Window.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;

	class JWModel
	{
	public:
		JWModel() = default;
		~JWModel();

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera) noexcept;

		void LoadModelObj(STRING Directory, STRING FileName) noexcept;
		
		void SetWorldMatrixToIdentity() noexcept;
		auto SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder Order) noexcept->JWModel&;

		auto SetTranslation(XMFLOAT3 Offset) noexcept->JWModel&;
		auto SetRotation(XMFLOAT4 RotationAxis, float Angle) noexcept->JWModel&;
		auto SetScale(XMFLOAT3 Scale) noexcept->JWModel&;

		auto GetWorldPosition() noexcept->XMVECTOR;
		auto GetDistanceFromCamera() noexcept->float;

		void Draw() noexcept;

	private:
		void CheckValidity() const noexcept;

		void CreateVertexBuffer() noexcept;
		void CreateIndexBuffer() noexcept;

		void CreateSamplerState() noexcept;

		void CreateTexture(WSTRING TextureFileName) noexcept;

		auto AddVertex(const SVertex& Vertex) noexcept->JWModel&;
		auto AddIndex(const SIndex& Index) noexcept->JWModel&;
		void AddEnd() noexcept;

		void UpdateWorldMatrix() noexcept;
		void Update() noexcept;

	private:
		bool m_IsValid{ false };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};

		SVertexData m_VertexData{};
		SIndexData m_IndexData{};

		ID3D11ShaderResourceView* m_TextureShaderResourceView{};
		ID3D11SamplerState* m_TextureSamplerState{};

		XMMATRIX m_WVP{};
		XMMATRIX m_MatrixWorld{};

		XMMATRIX m_MatrixTranslation{};
		XMMATRIX m_MatrixRotation{};
		XMMATRIX m_MatrixScale{};

		XMVECTOR m_WorldPosition{};

		EWorldMatrixCalculationOrder m_CalculationOrder{ EWorldMatrixCalculationOrder::ScaleRotTrans };
	};
};

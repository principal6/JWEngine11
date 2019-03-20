#pragma once

#include "JWCommon.h"

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

		void ShouldDrawNormals(bool Value) noexcept;

		auto GetWorldPosition() noexcept->XMVECTOR;
		auto GetDistanceFromCamera() noexcept->float;

		// Called in JWGame class
		void Draw() noexcept;

	private:
		void CheckValidity() const noexcept;

		void CreateTexture(WSTRING TextureFileName) noexcept;

		auto AddVertex(const SVertex& Vertex) noexcept->JWModel&;
		auto AddIndex(const SIndex& Index) noexcept->JWModel&;
		void AddEnd() noexcept;

		auto NormalAddVertex(const SVertex& Vertex) noexcept->JWModel&;
		auto NormalAddIndex(const SIndex2& Index) noexcept->JWModel&;
		void NormalAddEnd() noexcept;
		void DrawNormals() noexcept;

		void UpdateWorldMatrix() noexcept;
		void UpdateModel() noexcept;
		void UpdateNormals() noexcept;

	private:
		bool m_IsValid{ false };
		BOOL m_HasTexture{ FALSE };
		bool m_ShouldDrawNormals{ false };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};

		SVertexData m_VertexData{};
		SIndexData m_IndexData{};

		ID3D11Buffer* m_NormalVertexBuffer{};
		ID3D11Buffer* m_NormalIndexBuffer{};

		SVertexData m_NormalVertexData{};
		SIndex2Data m_NormalIndexData{};

		ID3D11ShaderResourceView* m_TextureShaderResourceView{};

		XMMATRIX m_MatrixWorld{};
		XMMATRIX m_MatrixTranslation{};
		XMMATRIX m_MatrixRotation{};
		XMMATRIX m_MatrixScale{};

		XMVECTOR m_WorldPosition{};

		EWorldMatrixCalculationOrder m_CalculationOrder{ EWorldMatrixCalculationOrder::ScaleRotTrans };

		SDefaultVSConstantBufferData m_DefaultVSConstantBufferData{};
		SDefaultPSConstantBufferData m_DefaultPSConstantBufferData{};
	};
};

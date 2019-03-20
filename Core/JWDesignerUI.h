#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;

	class JWDesignerUI
	{
	public:
		JWDesignerUI() = default;
		~JWDesignerUI();

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera) noexcept;

		// Called in JWGame class
		void Draw() noexcept;

	private:
		void MakeGrid(float XSize, float ZSize, float GridInterval = 1.0f) noexcept;

		auto AddVertex(const SVertexColor& Vertex) noexcept->JWDesignerUI&;
		auto AddIndex(const SIndex2& Index) noexcept->JWDesignerUI&;
		void AddEnd() noexcept;
		
		void Update() noexcept;

	private:
		static constexpr float KAxisLength = 1000.0f;
		static constexpr XMFLOAT4 KXAxisColor = XMFLOAT4(1, 0, 0, 1); // X = R
		static constexpr XMFLOAT4 KYAxisColor = XMFLOAT4(0, 1, 0, 1); // Y = G
		static constexpr XMFLOAT4 KZAxisColor = XMFLOAT4(0, 0, 1, 1); // Z = B
		bool m_IsValid{ false };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};

		SVertexColorData m_VertexData{};
		SIndex2Data m_IndexData{};

		XMMATRIX m_MatrixWorld{};

		SColorVSConstantBufferData m_ColorVSConstantBufferData{};
	};
};

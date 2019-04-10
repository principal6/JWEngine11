#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	
	class JWLine final
	{
	public:
		JWLine() = default;
		~JWLine();

		// Called in JWGame class
		void Create(JWDX& DX) noexcept;

		void Make3DGrid(float XSize, float ZSize, float GridInterval) noexcept;

		void AddLine3D(XMFLOAT3 StartPosition, XMFLOAT3 EndPosition, XMFLOAT4 Color) noexcept;
		void AddLine2D(XMFLOAT2 StartPosition, XMFLOAT2 Length, XMFLOAT4 Color) noexcept;
		void AddEnd() noexcept;

		void SetLine3D(size_t Line3DIndex, XMFLOAT3 StartPosition, XMFLOAT3 EndPosition, XMFLOAT4 Color) noexcept;
		void SetLine2D(size_t Line2DIndex, XMFLOAT2 StartPosition, XMFLOAT2 Length, XMFLOAT4 Color) noexcept;

		void UpdateLines() noexcept;

	public:
		static constexpr float		KAxisLength = 1000.0f;
		static constexpr XMFLOAT4	KXAxisColor = XMFLOAT4(1, 0, 0, 1); // X = R
		static constexpr XMFLOAT4	KYAxisColor = XMFLOAT4(0, 1, 0, 1); // Y = G
		static constexpr XMFLOAT4	KZAxisColor = XMFLOAT4(0, 0, 1, 1); // Z = B

		bool					m_IsValid{ false };

		JWDX*					m_pDX{};

		ID3D11Buffer*			m_VertexBuffer{};
		ID3D11Buffer*			m_IndexBuffer{};
		SVertexDataStaticModel	m_VertexData{};
		SIndexDataLine			m_IndexData{};

		SVSCBSpace				m_VSCBSpace{};

		ERenderType				m_RenderType{ ERenderType::Invalid };
	};
};

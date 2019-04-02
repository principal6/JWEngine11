#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;
	
	class JWLine final
	{
	public:
		JWLine() = default;
		~JWLine();

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera) noexcept;

		void AddLine(SLineData LineData) noexcept;
		void AddEnd() noexcept;

		void SetLine(size_t LineIndex, SLineData LineData) noexcept;

		void UpdateLines() noexcept;

		// Called in JWGame class
		void Draw() noexcept;

	private:
		void CheckValidity() const noexcept;

		// Called by UpdateVertices()
		void UpdateVertexBuffer() noexcept;

		// Called by Draw()
		void Update() noexcept;

	private:
		bool m_IsValid{ false };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};

		SStaticModelVertexData m_VertexData{};
		SLineIndexData m_IndexData{};

		XMFLOAT2 m_Position{};
		XMFLOAT2 m_Size{};

		SVSCBStatic m_VSCBStatic{};
	};
};

#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;

	class JWImage
	{
	public:
		JWImage() = default;
		~JWImage() = default;

		void Create(JWDX& DX, const SSize2& WindowSize) noexcept;
		void Destroy() noexcept;

		auto SetPosition(XMFLOAT2 Position) noexcept->JWImage*;
		auto SetSize(XMFLOAT2 Size) noexcept->JWImage*;

		auto UpdatePositionAndSize() noexcept->JWImage*;

	public:
		ID3D11Buffer*				m_VertexBuffer{};
		ID3D11Buffer*				m_IndexBuffer{};
		SVertexDataModel			m_VertexData{};
		SIndexDataTriangle			m_IndexData{};

	private:
		bool						m_IsCreated{ false };
		JWDX*						m_pDX{};
		const SSize2*				m_pWindowSize{};

		XMFLOAT2					m_Position{};
		XMFLOAT2					m_Size{};
	};
};

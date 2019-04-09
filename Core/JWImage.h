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

		void Create(JWDX& DX) noexcept;
		void Destroy() noexcept;

		auto SetPosition(XMFLOAT2 Position) noexcept->JWImage*;
		auto SetSize(XMFLOAT2 Size) noexcept->JWImage*;

	public:
		ID3D11Buffer*				m_VertexBuffer{};
		ID3D11Buffer*				m_IndexBuffer{};
		SStaticModelVertexData		m_VertexData{};
		SModelIndexData				m_IndexData{};

	private:
		void UpdateScreenPositionAndSize() noexcept;

	private:
		bool						m_IsValid{ false };
		JWDX*						m_pDX{};
		XMFLOAT2					m_Position{};
		XMFLOAT2					m_Size{};
	};
};

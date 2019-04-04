#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;

	class JWImage
	{
	public:
		JWImage() = default;
		~JWImage();

		void Create(JWDX& DX, JWCamera& Camera) noexcept;

		void SetPosition(XMFLOAT2 Position) noexcept;
		void SetSize(XMFLOAT2 Size) noexcept;

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
		JWCamera*					m_pCamera{};
		XMFLOAT2					m_Position{};
		XMFLOAT2					m_Size{};
	};
};

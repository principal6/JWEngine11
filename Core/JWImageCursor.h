#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;

	class JWImageCursor
	{
	public:
		JWImageCursor() = default;
		~JWImageCursor();

		void Create(JWDX& DX, const SSize2& WindowSize) noexcept;

		void LoadImageCursorFromFile(STRING Directory, STRING FileName) noexcept;

		auto SetPosition(XMFLOAT2 Position) noexcept->JWImageCursor&;

		void Draw() noexcept;

	protected:
		// Called by SetPosition()
		void UpdateVertices() noexcept;

	protected:
		bool						m_IsCreated{ false };
		bool						m_IsTextureCreated{ false };

		JWDX*						m_pDX{};
		const SSize2*				m_pWindowSize{};

		ID3D11Buffer*				m_VertexBuffer{};
		ID3D11Buffer*				m_IndexBuffer{};
		SVertexDataModel			m_VertexData{};
		SIndexDataTriangle			m_IndexData{};
		ID3D11ShaderResourceView*	m_TextureShaderResourceView{};

		XMFLOAT2					m_Position{};
		XMFLOAT2					m_Size{};
		SSize2					m_OriginalSize{};

		SVSCBSpace					m_VSCBSpace{};
	};
};

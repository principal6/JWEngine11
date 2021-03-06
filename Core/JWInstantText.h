#pragma once

#include "JWCommon.h"
#include "JWBMFontParser.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	
	static constexpr uint32_t	KMaxInsantTextLength = 2048;
	
	class JWInstantText
	{
	public:
		JWInstantText() = default;
		~JWInstantText() = default;

		void Create(JWDX& DX, const SSize2& WindowSize, STRING BaseDirectory, STRING FontFileName) noexcept;
		void Destroy() noexcept;

		void BeginRendering() noexcept;
		void RenderText(const WSTRING& Text, XMFLOAT2 Position, XMFLOAT4 FontColorRGB) noexcept;
		void EndRendering() noexcept;

	private:
		void CreateInstantTextVertexBuffer() noexcept;
		void CreateInstantTextIndexBuffer() noexcept;

		inline void LoadImageFromFile(STRING Directory, STRING FileName) noexcept;

	private:
		bool						m_IsCreated{ false };

		JWDX*						m_pDX{};
		const SSize2*				m_pWindowSize{};
		STRING						m_BaseDirectory{};

		JWBMFontParser				m_FontParser{};

		ID3D11Buffer*				m_VertexBuffer{};
		ID3D11Buffer*				m_IndexBuffer{};
		SVertexDataText				m_VertexData{};
		SIndexDataTriangle			m_IndexData{};

		SVSCBSpace					m_VSCBSpace{};

		ID3D11ShaderResourceView*	m_FontTextureSRV{};
		uint32_t					m_TotalTextLength{};

		bool						m_ShouldToggleWireFrame{ false };
	};
}
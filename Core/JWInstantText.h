#pragma once

#include "JWCommon.h"
#include "JWBMFontParser.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;
	
	static constexpr uint32_t KMaxInsantTextLength = 2048;
	
	class JWInstantText
	{
	public:
		JWInstantText() = default;
		~JWInstantText();

		void Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory, STRING FontFileName) noexcept;

		void BeginRendering() noexcept;
		void RenderText(const WSTRING& Text, XMFLOAT2 Position, XMFLOAT4 FontColorRGB) noexcept;
		void EndRendering() noexcept;

	private:
		void CreateInstantTextVertexBuffer() noexcept;
		void CreateInstantTextIndexBuffer() noexcept;
		void CreateInstantTextVS() noexcept;
		void CreateInstantTextPS() noexcept;

		inline void LoadImageFromFile(STRING Directory, STRING FileName) noexcept;

	private:
		bool						m_IsValid{ false };

		JWDX*						m_pDX{};
		JWCamera*					m_pCamera{};

		STRING						m_BaseDirectory{};
		JWBMFontParser				m_FontParser{};

		ID3D11Buffer*				m_VertexBuffer{};
		ID3D11Buffer*				m_IndexBuffer{};
		SVertexDataText				m_VertexData{};
		SIndexDataTriangle			m_IndexData{};

		SVSCBSpace					m_VSCBSpace{};
		ID3D11InputLayout*			m_IAInputLayoutText{};
		ID3D10Blob*					m_VSInstantTextBlob{};
		ID3D11VertexShader*			m_VSInstantText{};
		ID3D10Blob*					m_PSInstantTextBlob{};
		ID3D11PixelShader*			m_PSInstantText{};

		ID3D11ShaderResourceView*	m_FontTextureSRV{};
		uint32_t					m_TotalTextLength{};
	};
}
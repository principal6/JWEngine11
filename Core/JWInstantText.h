#pragma once

#include "JWCommon.h"
#include "JWBMFontParser.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;
	
	static constexpr uint16_t KMaxInsantTextLength = 256;

	struct SPSInstantTextCBData
	{
		SPSInstantTextCBData() {};
		SPSInstantTextCBData(XMFLOAT4 __RGBA) : _RGBA(__RGBA) {};

		XMFLOAT4 _RGBA{};
	};
	
	class JWInstantText
	{
	public:
		JWInstantText() = default;
		~JWInstantText();

		void Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory, STRING FontFileName) noexcept;

		void DrawInstantText(STRING Text, XMFLOAT2 Position, XMFLOAT3 FontColorRGB) noexcept;

	private:
		void CreateInstantTextVertexBuffer() noexcept;
		void CreateInstantTextIndexBuffer() noexcept;
		void CreateInstantTextPS() noexcept;
		void CreatePSConstantBuffer() noexcept;

		void LoadImageFromFile(STRING Directory, STRING FileName) noexcept;

	private:
		bool m_IsValid{ false };
		bool m_IsTextureCreated{ false };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		STRING m_BaseDirectory{};
		JWBMFontParser m_FontParser{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};
		SStaticModelVertexData m_VertexData{};
		SModelIndexData m_IndexData{};

		SVSCBStatic	m_VSCBStatic{};
		ID3D11PixelShader* m_PSInstantText{};
		ID3D11Buffer* m_PSInstantTextCB{};

		SPSInstantTextCBData m_TextColor{};
		ID3D11ShaderResourceView* m_TextureShaderResourceView{};
	};
}
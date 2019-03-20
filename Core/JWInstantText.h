#pragma once

#include "JWBMFontParser.h"
#include "JWImage.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;

	struct SInstantTextPSConstantBufferData
	{
		SInstantTextPSConstantBufferData() {};
		SInstantTextPSConstantBufferData(XMFLOAT4 __RGBA) : _RGBA(__RGBA) {};

		XMFLOAT4 _RGBA{};
	};
	
	class JWInstantText final : protected JWImage
	{
	public:
		~JWInstantText();

		void Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory, STRING FontFileName) noexcept;

		void SetInstantTextPS() noexcept;

		void DrawInstantText(STRING Text, XMFLOAT2 Position, XMFLOAT3 FontColorRGB) noexcept;

	private:
		void CreateInstantTextVertexBuffer() noexcept;
		void CreateInstantTextIndexBuffer() noexcept;
		void CreateInstantTextPS() noexcept;
		void CreatePSConstantBuffer() noexcept;

	private:
		static constexpr int KMaxInsantTextLength = 256;

		STRING m_BaseDirectory;

		JWBMFontParser m_FontParser;

		ID3D11PixelShader* m_InstantTextPS{};

		ID3D11Buffer* m_InstantTextPSConstantBuffer{};
		SInstantTextPSConstantBufferData m_TextColor{};
	};
}
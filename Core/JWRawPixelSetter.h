#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;

	struct SRawPixelColor
	{
		byte B{};
		byte G{};
		byte R{};
		byte A{};

		SRawPixelColor() = default;
		// @important:
		// We use DXGI_FORMAT_B8G8R8A8_UNORM for our texture
		// so the computer recieves the data in B-G-R-A order,
		// but for human R-G-B-A is easier to understand.
		// So I used R-G-B-A order.
		// Each color value ranges [0, 255].
		SRawPixelColor(byte _R, byte _G, byte _B, byte _A) : B(_B), G(_G), R(_R), A(_A) {};
	};

	struct SRawPixelData
	{
		VECTOR<SRawPixelColor> vPixels;

		int Width{}, Height{};
		int PixelCount{};

		void CreatePixelData(int screen_width, int screen_height)
		{
			Width = screen_width;
			Height = screen_height;

			PixelCount = screen_width * screen_height;

			vPixels.reserve(PixelCount);
			vPixels.resize(PixelCount);

			ClearData();
		}

		void SetPixel(int X, int Y, SRawPixelColor Color)
		{
			int index = X + Y * Width;
			vPixels[index] = Color;
		}

		void FillRect(SPositionInt Position, SSizeInt Size, SRawPixelColor Color)
		{
			uint32_t color_byte4 = (Color.B << 24 | Color.G << 16 | Color.R << 8 | Color.A);

			for (uint32_t y = 0; y < Size.Height; ++y)
			{
				uint32_t start_id = Position.X + (y + Position.Y) * Width;
				uint32_t copy_byte_size = 4;
				uint32_t index{};

				for (uint32_t x = 0; x < Size.Width; ++x)
				{
					index = start_id + x;
					memcpy(&vPixels[index], &color_byte4, copy_byte_size);
				}
			}
		}

		auto GetWidth() { return Width; };
		auto GetHeight() { return Height; };
		const auto GetPtrData() { return &vPixels[0]; };
		auto GetByteSize() { return vPixels.size() * sizeof(SRawPixelColor); };
		void ClearData() { memset(GetPtrData(), 0, GetByteSize()); };
	};

	class JWRawPixelSetter
	{
	public:
		JWRawPixelSetter() = default;
		~JWRawPixelSetter();

		// Called in JWGame class
		void Create(JWDX& DX) noexcept;

		// Called in JWGame class
		auto GetRawPixelData() noexcept->SRawPixelData&;

		// Called in JWGame class
		void Draw() noexcept;

	private:
		// Called by Create()
		void CreateRawTexture() noexcept;

		// Called by Draw()
		void UpdateRawTexture() noexcept;

	private:
		bool m_IsCreated{ false };

		JWDX*						m_pDX{};

		SRawPixelData				m_RawPixelData{};

		ID3D11Texture2D*			m_RawTexture2D{};
		ID3D11ShaderResourceView*	m_RawTexture2DSRV{};
	};
};

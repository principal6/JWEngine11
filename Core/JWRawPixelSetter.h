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
		VECTOR<SRawPixelColor> Pixels;

		int Width{}, Height{};
		int PixelCount{};

		void CreatePixelData(int screen_width, int screen_height)
		{
			Width = screen_width;
			Height = screen_height;

			PixelCount = screen_width * screen_height;

			Pixels.reserve(PixelCount);
			Pixels.resize(PixelCount);

			ClearData();
		}

		void SetPixel(int X, int Y, SRawPixelColor Color)
		{
			Pixels[X + (Y * Width)] = Color;
		}

		void FillRect(SPositionInt Position, SSizeInt Size, SRawPixelColor Color)
		{
			int32_t color_byte4 = (Color.B << 24 | Color.G << 16 | Color.R << 8 | Color.A);

			for (int y = 0; y < Size.Height; ++y)
			{
				int start_id = Position.X + (y + Position.Y) * Width;
				int copy_byte_size = 4;

				for (int x = 0; x < Size.Width; ++x)
				{
					memcpy(&Pixels[start_id + x], &color_byte4, copy_byte_size);
				}
			}
		}

		auto GetWidth() { return Width; };
		auto GetHeight() { return Height; };
		const auto GetPtrData() { return &Pixels[0]; };
		auto GetByteSize() { return Pixels.size() * sizeof(SRawPixelColor); };
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
		bool m_IsValid{ false };

		JWDX* m_pDX{};

		SRawPixelData m_RawPixelData{};

		ID3D11Texture2D* m_RawTexture2D;
		ID3D11ShaderResourceView* m_RawTexture2DSRV;
	};
};
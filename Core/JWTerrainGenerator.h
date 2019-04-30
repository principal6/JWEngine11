#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWDX;

	class JWTerrainGenerator
	{
	public:
		void Create(JWDX& DX, const STRING& BaseDirectory) noexcept;
		void Destroy() noexcept {};

		// Supported format:
		// TIF(R8G8B8, non-compressed, non-layered)
		// TIF(R8, non-compressed, non-layered)
		auto GenerateTerrainFromFile(const STRING& FileName, float HeightFactor = 1.0f) noexcept->SModelData;

		inline auto ConvertR8G8B8ToFloat(unsigned char R, unsigned char G, unsigned char B, float division_factor) noexcept->float;
		inline auto ConvertR8ToFloat(unsigned char R, float division_factor) noexcept->float;

	private:
		void LoadGray8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
			float HeightFactor, SModelData& OutModelData) noexcept;
		void LoadR8G8B8A8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
			float HeightFactor, SModelData& OutModelData) noexcept;
		
	private:
		JWDX*	m_pDX{};
		STRING	m_BaseDirectory{};
	};
};
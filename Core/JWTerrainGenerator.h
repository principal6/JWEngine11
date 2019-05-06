#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// @warning:
	// If MaximumNodeSize is too small, it takes longer to generate the terrain!
	static constexpr int KMaximumNodeSize = 16;
	static constexpr int KMinimumNodeSize = 2;

	static constexpr int KMaxVertexMapIDCount = 4;

	struct SVertexMapEntry
	{
		int32_t VertexID[KMaxVertexMapIDCount]{ -1, -1, -1, -1 };
		uint32_t VertexCount{};

		void AddVertexID(int32_t ID)
		{
			if (VertexCount >= KMaxVertexMapIDCount)
			{
				return;
			}

			VertexID[VertexCount] = ID;
			++VertexCount;
		}
	};

	using SVertexMap = VECTOR<SVertexMapEntry>;

	class JWDX;

	class JWTerrainGenerator
	{
	public:
		void Create(JWDX& DX, const STRING& BaseDirectory) noexcept;
		void Destroy() noexcept {};

		// Supported format:
		// TIF(R8G8B8, non-compressed, non-layered)
		// TIF(R8, non-compressed, non-layered)
		auto GenerateTerrainFromHeightMap(const STRING& HeightMapFN, float HeightFactor = 1.0f, float XYSizeFactor = 1.0f) noexcept->STerrainData;

		void SaveTerrainAsTRN(const STRING& TRNFileName, const STerrainData& TerrainData) noexcept;

		auto LoadTerrainFromTRN(const STRING& TRNFileName) noexcept->STerrainData;

		inline auto ConvertR8G8B8ToFloat(unsigned char R, unsigned char G, unsigned char B, float factor) noexcept->float;
		inline auto ConvertR8ToFloat(unsigned char R, float factor) noexcept->float;
		inline auto ConvertR16ToFloat(unsigned short R, float factor) noexcept->float;

	private:
		// Gray scale 8-bit
		void LoadR8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
			float HeightFactor, float XYSizeFactor, SModelData& OutModelData, SVertexMap& OutVertexMap) noexcept;
		// Gray scale 16-bit
		void LoadR16UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
			float HeightFactor, float XYSizeFactor, SModelData& OutModelData, SVertexMap& OutVertexMap) noexcept;
		// RGB 8-bit
		void LoadR8G8B8A8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
			float HeightFactor, float XYSizeFactor, SModelData& OutModelData, SVertexMap& OutVertexMap) noexcept;

		void BuildQuadTree(STerrainData& TerrainData, int32_t CurrentNodeID) noexcept;
		void BuildQuadTreeMesh(STerrainData& TerrainData, const SModelData& ModelData) noexcept;
		
	private:
		JWDX*	m_pDX{};
		STRING	m_BaseDirectory{};
	};
};
#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// @warning:
	// If MaximumNodeSize is too small, it takes really long to generate the terrain!
	static constexpr int KMaximumNodeSizeX = 32;
	static constexpr int KMaximumNodeSizeZ = 32;
	static constexpr int KMinimumNodeSizeX = 2;
	static constexpr int KMinimumNodeSizeZ = 2;

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
		auto GenerateTerrainFromFile(const STRING& FileName, float HeightFactor = 1.0f) noexcept->STerrainData;

		inline auto ConvertR8G8B8ToFloat(unsigned char R, unsigned char G, unsigned char B, float division_factor) noexcept->float;
		inline auto ConvertR8ToFloat(unsigned char R, float division_factor) noexcept->float;

	private:
		void LoadGray8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
			float HeightFactor, SModelData& OutModelData, SVertexMap& OutVertexMap) noexcept;
		void LoadR8G8B8A8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
			float HeightFactor, SModelData& OutModelData, SVertexMap& OutVertexMap) noexcept;

		void BuildQuadTree(STerrainData& TerrainData, int32_t CurrentNodeID) noexcept;
		void BuildQuadTreeMesh(STerrainData& TerrainData, const SModelData& ModelData) noexcept;
		
	private:
		JWDX*	m_pDX{};
		STRING	m_BaseDirectory{};
	};
};
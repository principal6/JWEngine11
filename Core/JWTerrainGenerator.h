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

		auto GenerateTerrainFromFile(const STRING& FileName, float HeightFactor = 1.0f) noexcept->SModelData;

	private:
		JWDX*	m_pDX{};
		STRING	m_BaseDirectory{};
	};
};
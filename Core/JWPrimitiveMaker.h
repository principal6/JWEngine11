#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWPrimitiveMaker
	{
	public:
		// Points A - B - C must be in clockwise order
		auto MakeTriangle(XMFLOAT3 A, XMFLOAT3 B, XMFLOAT3 C) noexcept->SModelData;

		auto MakeSquare(float Size, XMFLOAT2 UVMap) noexcept->SModelData;

		// Minimum detail is 5
		auto MakeCircle(float Radius, uint8_t Detail) noexcept->SModelData;

		auto MakeCube(float Size) noexcept->SModelData;
		auto MakePyramid(float Height, float Width) noexcept->SModelData;

		// Minimum detail is 5
		auto MakeCone(float Height, float Radius, uint8_t Detail) noexcept->SModelData;

		// Minimum detail is 5
		auto MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept->SModelData;

		// Minimum vertical detail is 4
		// Minimum horizontal detail is 1
		auto MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->SModelData;

		// Minimum vertical detail is 4
		// Minimum horizontal detail is 1
		// If horizontal detail input is an even number, it automatically changes to be an odd number by adding 1 to it.
		// This is because even numbered horizontal detail can cause crack in the capsule.
		auto MakeCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->SModelData;
	};
}
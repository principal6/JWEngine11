#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWPrimitiveMaker
	{
	public:
		auto MakeSquare(float Size, XMFLOAT2 UVMap) noexcept->SStaticModelData;

		// Minimum detail is 5
		auto MakeCircle(float Radius, uint8_t Detail) noexcept->SStaticModelData;

		auto MakeCube(float Size) noexcept->SStaticModelData;
		auto MakePyramid(float Height, float Width) noexcept->SStaticModelData;

		// Minimum detail is 5
		auto MakeCone(float Height, float Radius, uint8_t Detail) noexcept->SStaticModelData;

		// Minimum detail is 5
		auto MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept->SStaticModelData;

		// Minimum vertical detail is 4
		// Minimum horizontal detail is 1
		auto MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->SStaticModelData;

		// Minimum vertical detail is 4
		// Minimum horizontal detail is 1
		// If horizontal detail input is an even number, it automatically changes to be an odd number by adding 1 to it.
		// This is because even numbered horizontal detail can cause crack in the capsule.
		auto MakeCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->SStaticModelData;
	};
}
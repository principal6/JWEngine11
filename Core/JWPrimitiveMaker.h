#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWPrimitiveMaker
	{
	public:
		// Points A - B - C must be in clockwise order
		auto MakeTriangle(const XMFLOAT3& A, const XMFLOAT3& B, const XMFLOAT3& C) noexcept->SModelData;

		// Points A - B - C - D must be in clockwise order
		auto MakeQuad(const XMFLOAT3& A, const XMFLOAT3& B, const XMFLOAT3& C, const XMFLOAT3& D) noexcept->SModelData;

		auto MakeSquare(float Size, XMFLOAT2 UVMap) noexcept->SModelData;

		// Minimum detail is 5
		auto MakeCircle(float Radius, uint8_t Detail) noexcept->SModelData;

		auto MakeCube(float Size) noexcept->SModelData;

		// NA~ND: (Front) Near plane 4 vertices
		// FA~FD: (Back) Far plane 4 vertices
		// Points ( NA - NB - NC - ND ) & ( FA - FB - FC - FD ) must be in clockwise order
		// Planes order: F - B - U - D - L - R
		auto MakeHexahedron(
			const XMFLOAT3& NA = XMFLOAT3(0, 0, 0), const XMFLOAT3& NB = XMFLOAT3(0, 0, 0),
			const XMFLOAT3& NC = XMFLOAT3(0, 0, 0), const XMFLOAT3& ND = XMFLOAT3(0, 0, 0),
			const XMFLOAT3& FA = XMFLOAT3(0, 0, 0), const XMFLOAT3& FB = XMFLOAT3(0, 0, 0),
			const XMFLOAT3& FC = XMFLOAT3(0, 0, 0), const XMFLOAT3& FD = XMFLOAT3(0, 0, 0)) noexcept->SModelData;

		auto MakePyramid(float Height, float Width) noexcept->SModelData;

		// Minimum detail is 5
		auto MakeCone(float Height, float Radius, uint8_t Detail) noexcept->SModelData;

		// Minimum detail is 5
		auto MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept->SModelData;

		// Minimum vertical detail is 4
		// Minimum horizontal detail is 1
		auto MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail,
			const XMFLOAT3& ColorTop = XMFLOAT3(1, 0, 0), const XMFLOAT3& ColorBottom = XMFLOAT3(0, 0, 1)) noexcept->SModelData;

		// Minimum vertical detail is 4
		// Minimum horizontal detail is 1
		// If horizontal detail input is an even number, it automatically changes to be an odd number by adding 1 to it.
		// This is because even numbered horizontal detail can cause crack in the capsule.
		auto MakeCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->SModelData;

	private:
		inline auto InterpolateColor(const XMFLOAT3& ColorA, const XMFLOAT3& ColorB, float d) noexcept->XMFLOAT3;
	};
}
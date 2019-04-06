#pragma once

#include "JWAssimpLoader.h"

namespace JWEngine
{
	class JWDX;

	class JWModel
	{
	public:
		JWModel() = default;
		~JWModel() = default;

		void Create(JWDX& DX, STRING& BaseDirectory) noexcept;
		
		// Release all resources
		void Destroy() noexcept;

		void MakeSquare(float Size, XMFLOAT2 UVMap) noexcept;

		// Minimum detail is 5
		void MakeCircle(float Radius, uint8_t Detail) noexcept;

		void MakeCube(float Size) noexcept;
		void MakePyramid(float Height, float Width) noexcept;

		// Minimum detail is 5
		void MakeCone(float Height, float Radius, uint8_t Detail) noexcept;

		// Minimum detail is 5
		void MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept;

		// Minimum vertical detail is 4
		// Minimum horizontal detail is 1
		void MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept;

		// Minimum vertical detail is 4
		// Minimum horizontal detail is 1
		// If horizontal detail input is an even number, it automatically changes to be an odd number by adding 1 to it.
		// This is because even numbered horizontal detail can cause crack in the capsule.
		void MakeCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept;

		void SetStaticModelData(SStaticModelData ModelData) noexcept;
		void SetRiggedModelData(SRiggedModelData ModelData) noexcept;
		void SetLineModelData(SLineModelData Model2Data) noexcept;

		// Load animation
		auto AddAnimationFromFile(STRING ModelFileName) noexcept->JWModel*;

		auto GetRenderType() const noexcept { return m_RenderType; };
		auto GetTextureFileName() const noexcept->WSTRING
		{
			if (m_RenderType == ERenderType::Model_Static)
			{
				return StaticModelData.TextureFileNameW;
			}
			else if (m_RenderType == ERenderType::Model_Rigged)
			{
				return RiggedModelData.TextureFileNameW;
			}

			return L"";
		}

	public:
		ID3D11Buffer*		ModelVertexBuffer{};
		ID3D11Buffer*		ModelIndexBuffer{};
		SStaticModelData	StaticModelData{};
		SRiggedModelData	RiggedModelData{};

		ID3D11Buffer*		NormalVertexBuffer{};
		ID3D11Buffer*		NormalIndexBuffer{};
		SLineModelData		NormalData{};

	private:
		void CreateModelVertexIndexBuffers() noexcept;

	private:
		JWDX*			m_pDX{};
		const STRING*	m_pBaseDirectory{};
		ERenderType		m_RenderType{ ERenderType::Invalid };
	};
};

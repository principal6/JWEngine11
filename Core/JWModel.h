#pragma once

#include "JWCommon.h"

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

		void SetStaticModelData(SStaticModelData ModelData) noexcept;
		void SetRiggedModelData(SRiggedModelData ModelData) noexcept;
		void SetLineModelData(SLineModelData Model2Data) noexcept;

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

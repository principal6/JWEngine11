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

		void SetNonRiggedModelData(SNonRiggedModelData ModelData) noexcept;
		void SetDynamicModelData(SNonRiggedModelData ModelData) noexcept;
		void SetRiggedModelData(SRiggedModelData ModelData) noexcept;

		// Only available when it's dynamic model
		auto SetVertex(uint32_t VertexIndex, XMFLOAT3 Position, XMFLOAT4 Color) noexcept->JWModel*;

		// Only available when it's dynamic model
		void UpdateModel() noexcept;

		auto GetRenderType() const noexcept { return m_RenderType; };
		auto GetTextureFileName() const noexcept->WSTRING
		{
			if (m_RenderType == ERenderType::Model_Static)
			{
				return NonRiggedModelData.TextureFileNameW;
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
		SNonRiggedModelData	NonRiggedModelData{};
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

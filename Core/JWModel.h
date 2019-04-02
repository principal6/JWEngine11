#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWDX;

	enum EFLAGRenderOption : uint8_t
	{
		JWFlagRenderOption_UseTexture = 0b1,
		JWFlagRenderOption_UseLighting = 0b10,
		JWFlagRenderOption_UseAnimationInterpolation = 0b100,
		JWFlagRenderOption_UseTransparency = 0b1000,
		JWFlagRenderOption_DrawNormals = 0b10000,
		JWFlagRenderOption_DrawTPose = 0b100000,
	};
	using JWFlagRenderOption = uint8_t;

	class JWModel
	{
	public:
		JWModel() = default;
		~JWModel();

		void Create(JWDX& DX) noexcept;

		void SetStaticModelData(SStaticModelData ModelData) noexcept;
		void SetRiggedModelData(SRiggedModelData ModelData) noexcept;
		void SetModel2Data(SModel2Data Model2Data) noexcept;

		// Load & Set animation
		auto AddAnimationFromFile(STRING ModelFileName) noexcept->JWModel&;
		auto SetAnimation(size_t AnimationID, bool ShouldRepeat = true) noexcept->JWModel&;
		auto SetPrevAnimation(bool ShouldRepeat = true) noexcept->JWModel&;
		auto SetNextAnimation(bool ShouldRepeat = true) noexcept->JWModel&;
		auto ToggleTPose() noexcept->JWModel&;
		
	private:
		void CreateTexture(WSTRING TextureFileName) noexcept;

		void CreateModelVertexIndexBuffers() noexcept;

		auto NormalAddVertex(const SStaticVertex& Vertex) noexcept->JWModel&;
		auto NormalAddIndex(const SIndex2& Index) noexcept->JWModel&;
		void NormalAddEnd() noexcept;

	public:
		JWFlagRenderOption FlagRenderOption{};
		
		ID3D11Buffer* ModelVertexBuffer{};
		ID3D11Buffer* ModelIndexBuffer{};
		SStaticModelData StaticModelData{};
		SRiggedModelData RiggedModelData{};

		ID3D11Buffer* NormalVertexBuffer{};
		ID3D11Buffer* NormalIndexBuffer{};
		SModel2Data NormalData{};

		ID3D11ShaderResourceView* TextureShaderResourceView{};

	private:
		JWDX* m_pDX{};

		EModelType m_ModelType{ EModelType::Invalid };
	};
};

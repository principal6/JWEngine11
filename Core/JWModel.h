#pragma once

#include "JWAssimpLoader.h"

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

		void MakeSquare(float Size) noexcept;

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

		void SetStaticModelData(SStaticModelData ModelData) noexcept;
		void SetRiggedModelData(SRiggedModelData ModelData) noexcept;
		void SetLineModelData(SLineModelData Model2Data) noexcept;

		// Load & Set animation
		auto AddAnimationFromFile(STRING Directory, STRING ModelFileName) noexcept->JWModel&;
		auto SetAnimation(size_t AnimationID, bool ShouldRepeat = true) noexcept->JWModel&;
		auto SetPrevAnimation(bool ShouldRepeat = true) noexcept->JWModel&;
		auto SetNextAnimation(bool ShouldRepeat = true) noexcept->JWModel&;
		auto ToggleTPose() noexcept->JWModel&;
		
	private:
		void CreateTexture(WSTRING TextureFileName) noexcept;

		void CreateModelVertexIndexBuffers() noexcept;

	public:
		ID3D11Buffer*		ModelVertexBuffer{};
		ID3D11Buffer*		ModelIndexBuffer{};
		SStaticModelData	StaticModelData{};
		SRiggedModelData	RiggedModelData{};

		ID3D11Buffer*		NormalVertexBuffer{};
		ID3D11Buffer*		NormalIndexBuffer{};
		SLineModelData		NormalData{};

		ID3D11ShaderResourceView*	TextureShaderResourceView{};

		JWFlagRenderOption			FlagRenderOption{};

	private:
		JWDX* m_pDX{};

		EModelType m_ModelType{ EModelType::Invalid };
	};
};

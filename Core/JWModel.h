#pragma once

#include "JWAssimpLoader.h"

namespace JWEngine
{
	class JWDX;

	enum class EModelType
	{
		Invalid,

		StaticModel,
		RiggedModel,
		LineModel,
	};

	class JWModel
	{
	public:
		JWModel() = default;
		~JWModel();

		void Create(JWDX& DX) noexcept;

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

		// Load & Set animation
		auto AddAnimationFromFile(STRING Directory, STRING ModelFileName) noexcept->JWModel&;
		auto SetAnimation(size_t AnimationID, bool ShouldRepeat = true) noexcept->JWModel&;
		auto SetPrevAnimation(bool ShouldRepeat = true) noexcept->JWModel&;
		auto SetNextAnimation(bool ShouldRepeat = true) noexcept->JWModel&;
		
	public:
		ID3D11Buffer*		ModelVertexBuffer{};
		ID3D11Buffer*		ModelIndexBuffer{};
		SStaticModelData	StaticModelData{};
		SRiggedModelData	RiggedModelData{};

		ID3D11Buffer*		NormalVertexBuffer{};
		ID3D11Buffer*		NormalIndexBuffer{};
		SLineModelData		NormalData{};

		ID3D11ShaderResourceView*	TextureShaderResourceView{};

	private:
		void CreateTexture(WSTRING TextureFileName) noexcept;
		void CreateModelVertexIndexBuffers() noexcept;

	private:
		JWDX*		m_pDX{};
		EModelType	m_ModelType{ EModelType::Invalid };
	};
};

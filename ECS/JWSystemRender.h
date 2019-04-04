#pragma once

#include "../Core/JWAssimpLoader.h"
#include "../Core/JWDX.h"
#include "../Core/JWModel.h"
#include "../Core/JWImage.h"

namespace JWEngine
{
	class JWCamera;
	class JWEntity;

	enum class ERenderType : uint8_t
	{
		Invalid,

		Model_StaticModel,
		Model_RiggedModel,

		Image_2D,
	};

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

	struct SComponentRender
	{
		JWEntity*			PtrEntity{};
		uint32_t			ComponentID{};

		ERenderType			RenderType{ ERenderType::Invalid };
		JWDX*				PtrDX{};
		JWCamera*			PtrCamera{};
		const STRING*		PtrBaseDirectory{};
		JWModel				Model{};
		JWImage				Image{};

		EDepthStencilState	DepthStencilState{ EDepthStencilState::ZEnabled };
		EVertexShader		VertexShader{ EVertexShader::VSBase };
		EPixelShader		PixelShader{ EPixelShader::PSBase };

		JWFlagRenderOption	FlagRenderOption{};

		auto SetTexture(ID3D11ShaderResourceView* pShaderResourceView) noexcept
		{
			JW_RELEASE(Model.TextureShaderResourceView);

			Model.TextureShaderResourceView = pShaderResourceView;

			FlagRenderOption |= JWFlagRenderOption_UseTexture;

			return this;
		}

		auto SetVertexShader(EVertexShader Shader) noexcept
		{
			VertexShader = Shader;
			return this;
		}

		auto SetPixelShader(EPixelShader Shader) noexcept
		{
			PixelShader = Shader;
			return this;
		}

		auto MakeSquare(float Size, XMFLOAT2 UVMap) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeSquare(Size, UVMap);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCircle(float Radius, uint8_t Detail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCircle(Radius, Detail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCube(float Size) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCube(Size);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakePyramid(float Height, float Width) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakePyramid(Height, Width);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCone(float Height, float Radius, uint8_t Detail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCone(Height, Radius, Detail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCylinder(Height, Radius, Detail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeSphere(Radius, VerticalDetail, HorizontalDetail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeCapsule(Height, Radius, VerticalDetail, HorizontalDetail);

				RenderType = ERenderType::Model_StaticModel;
			}

			return this;
		}

		auto LoadModel(ERenderType Type, STRING FileName) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				JWAssimpLoader loader;

				switch (Type)
				{
				case JWEngine::ERenderType::Model_StaticModel:
					Model.Create(*PtrDX);
					Model.SetStaticModelData(loader.LoadStaticModel(*PtrBaseDirectory + KAssetDirectory, FileName));

					RenderType = Type;
					break;
				case JWEngine::ERenderType::Model_RiggedModel:
					Model.Create(*PtrDX);
					Model.SetRiggedModelData(loader.LoadRiggedModel(*PtrBaseDirectory + KAssetDirectory, FileName));

					// @important
					VertexShader = EVertexShader::VSAnim;

					RenderType = Type;
					break;
				default:
					break;
				}

				
			}

			return this;
		}

		auto MakeImage2D(SPositionInt Position, SSizeInt Size) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Image.Create(*PtrDX, *PtrCamera);

				Image.SetPosition(XMFLOAT2(static_cast<float>(Position.X), static_cast<float>(Position.Y)));
				Image.SetSize(XMFLOAT2(static_cast<float>(Size.Width), static_cast<float>(Size.Height)));

				DepthStencilState = EDepthStencilState::ZDisabled;
				RenderType = ERenderType::Image_2D;
			}

			return this;
		}

		auto SetRenderFlag(JWFlagRenderOption Flag)
		{
			FlagRenderOption = Flag;

			return this;
		}

		auto ToggleRenderFlag(JWFlagRenderOption Flag)
		{
			FlagRenderOption ^= Flag;

			return this;
		}

		auto AddAnimation(STRING FileName)
		{
			Model.AddAnimationFromFile(*PtrBaseDirectory + KAssetDirectory, FileName);

			return this;
		}

		auto SetAnimation(uint32_t AnimationID)
		{
			Model.SetAnimation(AnimationID);

			return this;
		}

		auto NextAnimation()
		{
			Model.SetNextAnimation();

			return this;
		}

		auto PrevAnimation()
		{
			Model.SetPrevAnimation();

			return this;
		}
	};

	class JWSystemRender
	{
	public:
		JWSystemRender() {};
		~JWSystemRender();

		void CreateSystem(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept;

		auto CreateComponent() noexcept->SComponentRender&;
		void DestroyComponent(SComponentRender& Component) noexcept;

		void Update() noexcept;

	private:
		void SetShaders(SComponentRender& Component) noexcept;
		void Animate(SComponentRender& Component) noexcept;

		void UpdateNodeAnimationIntoBones(bool UseInterpolation, float AnimationTime, SRiggedModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void UpdateNodeTPoseIntoBones(float AnimationTime, SRiggedModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;

		void Draw(SComponentRender& Component) noexcept;
		void DrawNormals(SComponentRender& Component) noexcept;

	private:
		VECTOR<SComponentRender*>	m_vpComponents;
		JWDX*						m_pDX{};
		JWCamera*					m_pCamera{};
		STRING						m_BaseDirectory{};
		SVSCBStatic					m_VSCBStatic{};
		SVSCBRigged					m_VSCBRigged{};
	};
};
#pragma once

#include "../Core/JWAssimpLoader.h"
#include "../Core/JWModel.h"
#include "../Core/JWDX.h"

namespace JWEngine
{
	class JWCamera;
	class JWEntity;

	enum class ERenderType : uint8_t
	{
		Invalid,

		Model_StaticModel,
		Model_RiggedModel,
	};

	struct SComponentRender
	{
		JWEntity*		PtrEntity{};
		uint32_t		ComponentID{};

		ERenderType		RenderType{ ERenderType::Invalid };
		JWDX*			PtrDX{};
		const STRING*	PtrBaseDirectory{};
		JWModel			Model{};

		auto MakeSquare(float Size) noexcept
		{
			if (RenderType == ERenderType::Invalid)
			{
				Model.Create(*PtrDX);
				Model.MakeSquare(Size);

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
					break;
				case JWEngine::ERenderType::Model_RiggedModel:
					Model.Create(*PtrDX);
					Model.SetRiggedModelData(loader.LoadRiggedModel(*PtrBaseDirectory + KAssetDirectory, FileName));
					break;
				default:
					break;
				}

				RenderType = Type;
			}

			return this;
		}

		auto SetRenderFlag(JWFlagRenderOption Flag)
		{
			Model.FlagRenderOption = Flag;

			return this;
		}

		auto ToggleRenderFlag(JWFlagRenderOption Flag)
		{
			Model.FlagRenderOption ^= Flag;

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
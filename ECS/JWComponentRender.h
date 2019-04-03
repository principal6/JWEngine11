#pragma once

#include "../Core/JWAssimpLoader.h"
#include "../Core/JWModel.h"
#include "../Core/JWDX.h"

namespace JWEngine
{
	class JWEntity;

	enum class EComponentRenderType : uint8_t
	{
		Invalid,

		Model_StaticModel,
		Model_RiggedModel,
	};

	class JWComponentRender
	{
		friend class JWSystemRender;

	public:
		auto MakeSquare(float Size) noexcept
		{
			if (m_ComponentRenderType == EComponentRenderType::Invalid)
			{
				m_Model.Create(*m_pDX);
				m_Model.MakeSquare(Size);

				m_ComponentRenderType = EComponentRenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCircle(float Radius, uint8_t Detail) noexcept
		{
			if (m_ComponentRenderType == EComponentRenderType::Invalid)
			{
				m_Model.Create(*m_pDX);
				m_Model.MakeCircle(Radius, Detail);

				m_ComponentRenderType = EComponentRenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCube(float Size) noexcept
		{
			if (m_ComponentRenderType == EComponentRenderType::Invalid)
			{
				m_Model.Create(*m_pDX);
				m_Model.MakeCube(Size);

				m_ComponentRenderType = EComponentRenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakePyramid(float Height, float Width) noexcept
		{
			if (m_ComponentRenderType == EComponentRenderType::Invalid)
			{
				m_Model.Create(*m_pDX);
				m_Model.MakePyramid(Height, Width);

				m_ComponentRenderType = EComponentRenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCone(float Height, float Radius, uint8_t Detail) noexcept
		{
			if (m_ComponentRenderType == EComponentRenderType::Invalid)
			{
				m_Model.Create(*m_pDX);
				m_Model.MakeCone(Height, Radius, Detail);

				m_ComponentRenderType = EComponentRenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept
		{
			if (m_ComponentRenderType == EComponentRenderType::Invalid)
			{
				m_Model.Create(*m_pDX);
				m_Model.MakeCylinder(Height, Radius, Detail);

				m_ComponentRenderType = EComponentRenderType::Model_StaticModel;
			}

			return this;
		}

		auto MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept
		{
			if (m_ComponentRenderType == EComponentRenderType::Invalid)
			{
				m_Model.Create(*m_pDX);
				m_Model.MakeSphere(Radius, VerticalDetail, HorizontalDetail);

				m_ComponentRenderType = EComponentRenderType::Model_StaticModel;
			}

			return this;
		}

		auto LoadModel(EComponentRenderType Type, STRING FileName) noexcept
		{
			if (m_ComponentRenderType == EComponentRenderType::Invalid)
			{
				JWAssimpLoader loader;

				switch (Type)
				{
				case JWEngine::EComponentRenderType::Model_StaticModel:
					m_Model.Create(*m_pDX);
					m_Model.SetStaticModelData(loader.LoadStaticModel(*m_pBaseDirectory + KAssetDirectory, FileName));
					break;
				case JWEngine::EComponentRenderType::Model_RiggedModel:
					m_Model.Create(*m_pDX);
					m_Model.SetRiggedModelData(loader.LoadRiggedModel(*m_pBaseDirectory + KAssetDirectory, FileName));
					break;
				default:
					break;
				}

				m_ComponentRenderType = Type;
			}

			return this;
		}

		auto SetRenderFlag(JWFlagRenderOption Flag)
		{
			m_Model.FlagRenderOption = Flag;

			return this;
		}

		auto ToggleRenderFlag(JWFlagRenderOption Flag)
		{
			m_Model.FlagRenderOption ^= Flag;

			return this;
		}

		auto AddAnimation(STRING FileName)
		{
			m_Model.AddAnimationFromFile(*m_pBaseDirectory + KAssetDirectory, FileName);

			return this;
		}

		auto SetAnimation(uint32_t AnimationID)
		{
			m_Model.SetAnimation(AnimationID);

			return this;
		}

		auto NextAnimation()
		{
			m_Model.SetNextAnimation();

			return this;
		}

		auto PrevAnimation()
		{
			m_Model.SetPrevAnimation();

			return this;
		}

	public:
		JWEntity* pEntity{};

	protected:
		// Called by System's CreateComponent()
		JWComponentRender(uint32_t ComponentID) : m_ComponentID{ ComponentID } {};
		~JWComponentRender() {};
	
	protected:
		uint32_t m_ComponentID{};

		EComponentRenderType m_ComponentRenderType{ EComponentRenderType::Invalid };

		JWDX* m_pDX{};
		const STRING* m_pBaseDirectory{};

		JWModel m_Model{};
	};
};
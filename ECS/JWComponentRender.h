#pragma once

#include "../Core/JWCommon.h"
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
			m_Model.AddAnimationFromFile(FileName);

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
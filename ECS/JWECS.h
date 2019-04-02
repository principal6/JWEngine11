#pragma once

#include "JWEntity.h"

namespace JWEngine
{
	class JWDX;
	class JWCamera;

	class JWECS final
	{
	public:
		JWECS() {};
		~JWECS()
		{
			if (m_vpEntities.size())
			{
				for (auto& iter : m_vpEntities)
				{
					JW_DELETE(iter);
				}
			}
		};

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept
		{
			// @important

			m_SystemRender.CreateSystem(DX, Camera, BaseDirectory);
			m_SystemLight.CreateSystem(DX);
		}

		auto CreateEntity() noexcept->JWEntity&
		{
			m_vpEntities.push_back(new JWEntity(this));
			return *m_vpEntities[m_vpEntities.size() - 1];
		}

		auto GetEntity(uint32_t index) noexcept->JWEntity*
		{
			JWEntity* result{};

			if (index < m_vpEntities.size())
			{
				result = m_vpEntities[index];
			}

			return result;
		}

		void DestroyEntity(uint32_t index) noexcept
		{
			if (index < m_vpEntities.size())
			{
				JW_DELETE(m_vpEntities[index]);
				
				uint32_t last_index = static_cast<uint32_t>(m_vpEntities.size() - 1);
				if (index < last_index)
				{
					m_vpEntities[index] = m_vpEntities[last_index];
					m_vpEntities[last_index] = nullptr;
				}

				m_vpEntities.pop_back();
			}
		}

		auto& SystemTransform() noexcept { return m_SystemTransform; }
		auto& SystemRender() noexcept { return m_SystemRender; }
		auto& SystemLight() noexcept { return m_SystemLight; }

		void UpdateAll() noexcept
		{
			m_SystemTransform.Update();

			m_SystemLight.Update();
			
			m_SystemRender.Update();
		}

	private:
		JWSystemTransform m_SystemTransform;
		JWSystemRender m_SystemRender;
		JWSystemLight m_SystemLight;

		VECTOR<JWEntity*> m_vpEntities;
	};
};


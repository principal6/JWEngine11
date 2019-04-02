#pragma once

#include "JWComponentLight.h"

namespace JWEngine
{
	class JWDX;

	class JWSystemLight
	{
		friend class JWEntity;

	public:
		JWSystemLight() {};
		~JWSystemLight();

		// Called in JWECS class
		void CreateSystem(JWDX& DX) noexcept;

		void Update() noexcept;

	protected:
		// Called in JWEntity class
		auto CreateComponent() noexcept->JWComponentLight&;

		// Called in JWEntity class
		void DestroyComponent(JWComponentLight& Component) noexcept;

	private:
		bool m_ShouldUpdate{ false };

		VECTOR<JWComponentLight*> m_vpComponents;

		JWDX* m_pDX{};

		SPSCBLights m_PSCSLights{};
	};
};
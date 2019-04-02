#pragma once

#include "JWComponentTransform.h"

namespace JWEngine
{
	class JWSystemTransform
	{
		friend class JWEntity;

	public:
		JWSystemTransform() {};
		~JWSystemTransform();

		auto CreateComponent() noexcept->JWComponentTransform&;
		void DestroyComponent(JWComponentTransform& Component) noexcept;

		void Update() noexcept;

	private:
		VECTOR<JWComponentTransform*> m_vpComponents;
	};
};
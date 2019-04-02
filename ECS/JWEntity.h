#pragma once

#include "JWSystemTransform.h"
#include "JWSystemRender.h"
#include "JWSystemLight.h"

namespace JWEngine
{
	class JWECS;

	class JWEntity final
	{
	public:
		// Called by ECS
		JWEntity(JWECS* pECS) : m_pECS{ pECS } {};

		// Destroy all the components this Entity has
		~JWEntity();

		auto CreateComponentTransform() noexcept->JWComponentTransform*;
		auto GetComponentTransform() noexcept { return m_pComponentTransform; };

		auto CreateComponentRender() noexcept->JWComponentRender*;
		auto GetComponentRender() noexcept { return m_pComponentRender; };

		auto CreateComponentLight() noexcept->JWComponentLight*;
		auto GetComponentLight() noexcept { return m_pComponentLight; };

	private:
		JWECS* m_pECS{};

		JWComponentTransform*	m_pComponentTransform{};
		JWComponentRender*		m_pComponentRender{};
		JWComponentLight*		m_pComponentLight{};
	};
};
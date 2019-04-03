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

		auto CreateComponentTransform() noexcept->SComponentTransform*;
		auto GetComponentTransform() noexcept { return m_pComponentTransform; };

		auto CreateComponentRender() noexcept->SComponentRender*;
		auto GetComponentRender() noexcept { return m_pComponentRender; };

		auto CreateComponentLight() noexcept->SComponentLight*;
		auto GetComponentLight() noexcept { return m_pComponentLight; };

	private:
		JWECS* m_pECS{};

		SComponentTransform*	m_pComponentTransform{};
		SComponentRender*		m_pComponentRender{};
		SComponentLight*		m_pComponentLight{};
	};
};
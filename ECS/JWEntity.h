#pragma once

#include "JWSystemTransform.h"
#include "JWSystemRender.h"
#include "JWSystemLight.h"

namespace JWEngine
{
	class JWECS;

	enum class EEntityType
	{
		UserDefined,
		Sky,
		Grid,
		PickingRay,
		PickedTriangle,
		MainSprite,
	};

	class JWEntity
	{
	public:
		// Called by ECS
		JWEntity(JWECS* pECS, STRING Name) : m_pECS{ pECS }, m_EntityName{ Name } {};
		JWEntity(JWECS* pECS, STRING Name, EEntityType Type) : m_pECS{ pECS }, m_EntityName{ Name }, m_EntityType{ Type } {};

		// Destroy all the components this Entity has
		~JWEntity();

		auto CreateComponentTransform() noexcept->SComponentTransform*;
		auto GetComponentTransform() noexcept { return m_pComponentTransform; };

		auto CreateComponentRender() noexcept->SComponentRender*;
		auto GetComponentRender() noexcept { return m_pComponentRender; };

		auto CreateComponentLight() noexcept->SComponentLight*;
		auto GetComponentLight() noexcept { return m_pComponentLight; };
		
		auto GetEntityType() const noexcept { return m_EntityType; };
		const auto& GetEntityName() const noexcept { return m_EntityName; };

	private:
		JWECS*					m_pECS{};
		STRING					m_EntityName{};
		EEntityType				m_EntityType{ EEntityType::UserDefined };

		SComponentTransform*	m_pComponentTransform{};
		SComponentRender*		m_pComponentRender{};
		SComponentLight*		m_pComponentLight{};
	};
};
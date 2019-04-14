#pragma once

#include "JWSystemTransform.h"
#include "JWSystemRender.h"
#include "JWSystemLight.h"

namespace JWEngine
{
	class JWECS;

	// @warning
	// I must update these values carefully!
	static constexpr uint32_t KUniquePredefinedEntityCount = 5;
	enum class EEntityType
	{
		// UNIQUE predefined entity types
		Sky,
		Grid,
		PickingRay,
		PickedTriangle,
		MainSprite,

		// NOT unique predefined entity type
		Light,
		Camera,

		// User-defined entity type
		UserDefined,
	};

#define JW_IS_UNIQUE_ENTITY_TYPE(type) ((type == EEntityType::Sky) || (type == EEntityType::Grid) || \
	(type == EEntityType::PickingRay) || (type == EEntityType::PickedTriangle) || \
	(type == EEntityType::MainSprite))

	class JWEntity
	{
	public:
		// Called by ECS
		JWEntity(JWECS* pECS, STRING Name) : m_pECS{ pECS }, m_EntityName{ Name } {};

		// Destroy all the components this Entity has
		~JWEntity();

		auto CreateComponentTransform() noexcept->SComponentTransform*;
		auto GetComponentTransform() noexcept { return m_pComponentTransform; };

		auto CreateComponentRender() noexcept->SComponentRender*;
		auto GetComponentRender() noexcept { return m_pComponentRender; };

		auto CreateComponentLight() noexcept->SComponentLight*;
		auto GetComponentLight() noexcept { return m_pComponentLight; };
		
		// Default entity type is EEntityType::UserDefined
		void SetEntityType(EEntityType Type) noexcept;
		auto GetEntityType() const noexcept { return m_EntityType; };

		auto GetEntityName() const noexcept { return m_EntityName; };

	private:
		JWECS*					m_pECS{};
		STRING					m_EntityName{};
		EEntityType				m_EntityType{ EEntityType::UserDefined };

		SComponentTransform*	m_pComponentTransform{};
		SComponentRender*		m_pComponentRender{};
		SComponentLight*		m_pComponentLight{};
	};
};
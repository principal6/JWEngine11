#pragma once

#include "JWSystemTransform.h"
#include "JWSystemRender.h"
#include "JWSystemLight.h"
#include "JWSystemPhysics.h"
#include "JWSystemCamera.h"

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
		JWEntity() = default;
		~JWEntity() = default;
		
		void Create(JWECS* pECS, const STRING& EntityName) noexcept;
		void Create(JWECS* pECS, const STRING& EntityName, EEntityType EntityType) noexcept;
		void Destroy() noexcept;

		auto CreateComponentTransform() noexcept->SComponentTransform*;
		inline auto GetComponentTransform() noexcept { return m_pComponentTransform; };

		auto CreateComponentLight() noexcept->SComponentLight*;
		inline auto GetComponentLight() noexcept { return m_pComponentLight; };

		auto CreateComponentPhysics() noexcept->SComponentPhysics*;
		inline auto GetComponentPhysics() noexcept { return m_pComponentPhysics; };

		auto CreateComponentCamera() noexcept->SComponentCamera*;
		inline auto GetComponentCamera() noexcept { return m_pComponentCamera; };

		auto CreateComponentRender() noexcept->SComponentRender*;
		inline auto GetComponentRender() noexcept { return m_pComponentRender; };
		
		inline auto GetEntityType() const noexcept { return m_EntityType; };
		inline const auto& GetEntityName() const noexcept { return m_EntityName; };

	private:
		JWECS*					m_pECS{};
		STRING					m_EntityName{};
		EEntityType				m_EntityType{ EEntityType::UserDefined };

		SComponentTransform*	m_pComponentTransform{};
		SComponentRender*		m_pComponentRender{};
		SComponentLight*		m_pComponentLight{};
		SComponentPhysics*		m_pComponentPhysics{};
		SComponentCamera*		m_pComponentCamera{};
	};
};
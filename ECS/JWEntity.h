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
		ViewFrustum,
		Point3D,
		MainTerrain,
	};

	class JWEntity
	{
	public:
		JWEntity(EntityIndexType EntityIndex) : m_EntityIndex{ EntityIndex } {};
		~JWEntity() = default;
		
		void Create(JWECS* pECS, const STRING& EntityName) noexcept;
		void Create(JWECS* pECS, const STRING& EntityName, EEntityType EntityType) noexcept;
		void Destroy() noexcept;

		auto CreateComponentTransform() noexcept->SComponentTransform*;
		auto GetComponentTransform() noexcept->SComponentTransform*;
		void SetComponentTransformIndex(ComponentIndexType ComponentIndex) noexcept { m_ComponentTransformIndex = ComponentIndex; };

		auto CreateComponentLight() noexcept->SComponentLight*;
		auto GetComponentLight() noexcept->SComponentLight*;
		void SetComponentLightIndex(ComponentIndexType ComponentIndex) noexcept { m_ComponentLightIndex = ComponentIndex; };

		auto CreateComponentPhysics() noexcept->SComponentPhysics*;
		auto GetComponentPhysics() noexcept->SComponentPhysics*;
		void SetComponentPhysicsIndex(ComponentIndexType ComponentIndex) noexcept { m_ComponentPhysicsIndex = ComponentIndex; };

		auto CreateComponentCamera() noexcept->SComponentCamera*;
		auto GetComponentCamera() noexcept->SComponentCamera*;
		void SetComponentCameraIndex(ComponentIndexType ComponentIndex) noexcept { m_ComponentCameraIndex = ComponentIndex; };

		auto CreateComponentRender() noexcept->SComponentRender*;
		auto GetComponentRender() noexcept->SComponentRender*;
		void SetComponentRenderIndex(ComponentIndexType ComponentIndex) noexcept { m_ComponentRenderIndex = ComponentIndex; };
		
		inline void SetEntityIndex(EntityIndexType NewIndex) noexcept { m_EntityIndex = NewIndex; };
		inline auto GetEntityIndex() const noexcept { return m_EntityIndex; };

		inline auto GetEntityType() const noexcept { return m_EntityType; };
		inline const auto& GetEntityName() const noexcept { return m_EntityName; };

	private:
		JWECS*				m_pECS{};

		EntityIndexType		m_EntityIndex{};
		STRING				m_EntityName{};
		EEntityType			m_EntityType{ EEntityType::UserDefined };

		ComponentIndexType	m_ComponentTransformIndex{ KInvalidComponentIndex };
		ComponentIndexType	m_ComponentRenderIndex{ KInvalidComponentIndex };
		ComponentIndexType	m_ComponentLightIndex{ KInvalidComponentIndex };
		ComponentIndexType	m_ComponentPhysicsIndex{ KInvalidComponentIndex };
		ComponentIndexType	m_ComponentCameraIndex{ KInvalidComponentIndex };
	};
};
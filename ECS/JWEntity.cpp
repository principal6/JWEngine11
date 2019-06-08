#include "JWEntity.h"
#include "JWECS.h"

using namespace JWEngine;

void JWEntity::Create(JWECS* pECS, const STRING& EntityName) noexcept
{
	m_pECS = pECS;
	m_EntityName = EntityName;
}

void JWEntity::Create(JWECS* pECS, const STRING& EntityName, EEntityType EntityType) noexcept
{
	m_pECS = pECS;
	m_EntityName = EntityName;
	m_EntityType = EntityType;
}

void JWEntity::Destroy() noexcept
{
	if (m_ComponentTransformIndex != KInvalidComponentIndex)
	{
		m_pECS->SystemTransform().DestroyComponent(m_ComponentTransformIndex);
	}

	if (m_ComponentLightIndex != KInvalidComponentIndex)
	{
		m_pECS->SystemLight().DestroyComponent(m_ComponentLightIndex);
	}

	if (m_ComponentPhysicsIndex != KInvalidComponentIndex)
	{
		m_pECS->SystemPhysics().DestroyComponent(m_ComponentPhysicsIndex);
	}

	if (m_ComponentCameraIndex != KInvalidComponentIndex)
	{
		m_pECS->SystemCamera().DestroyComponent(m_ComponentCameraIndex);
	}

	if (m_ComponentRenderIndex != KInvalidComponentIndex)
	{
		m_pECS->SystemRender().DestroyComponent(m_ComponentRenderIndex);
	}
}

auto JWEntity::CreateComponentTransform() noexcept->SComponentTransform*
{
	m_ComponentTransformIndex = m_pECS->SystemTransform().CreateComponent(m_EntityIndex);

	return GetComponentTransform();
}

inline auto JWEntity::GetComponentTransform() noexcept->SComponentTransform*
{
	return m_pECS->SystemTransform().GetComponentPtr(m_ComponentTransformIndex);
}

auto JWEntity::CreateComponentLight() noexcept->SComponentLight*
{
	m_ComponentLightIndex = m_pECS->SystemLight().CreateComponent(m_EntityIndex);

	return GetComponentLight();
}

inline auto JWEntity::GetComponentLight() noexcept->SComponentLight*
{
	return m_pECS->SystemLight().GetComponentPtr(m_ComponentLightIndex);
}

auto JWEntity::CreateComponentPhysics() noexcept->SComponentPhysics*
{
	m_ComponentPhysicsIndex = m_pECS->SystemPhysics().CreateComponent(m_EntityIndex);

	return GetComponentPhysics();
}

inline auto JWEntity::GetComponentPhysics() noexcept->SComponentPhysics*
{
	return m_pECS->SystemPhysics().GetComponentPtr(m_ComponentPhysicsIndex);
}

auto JWEntity::CreateComponentCamera() noexcept->SComponentCamera*
{
	m_ComponentCameraIndex = m_pECS->SystemCamera().CreateComponent(m_EntityIndex);

	return GetComponentCamera();
}

inline auto JWEntity::GetComponentCamera() noexcept->SComponentCamera*
{
	return m_pECS->SystemCamera().GetComponentPtr(m_ComponentCameraIndex);
}

auto JWEntity::CreateComponentRender() noexcept->SComponentRender*
{
	m_ComponentRenderIndex = m_pECS->SystemRender().CreateComponent(m_EntityIndex);

	return GetComponentRender();
}

inline auto JWEntity::GetComponentRender() noexcept->SComponentRender*
{
	return m_pECS->SystemRender().GetComponentPtr(m_ComponentRenderIndex);
}
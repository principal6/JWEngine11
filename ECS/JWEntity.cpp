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
	if (m_pComponentTransform)
	{
		m_pECS->SystemTransform().DestroyComponent(*m_pComponentTransform);
	}

	if (m_pComponentLight)
	{
		m_pECS->SystemLight().DestroyComponent(*m_pComponentLight);
	}

	if (m_pComponentPhysics)
	{
		m_pECS->SystemPhysics().DestroyComponent(*m_pComponentPhysics);
	}

	if (m_pComponentCamera)
	{
		m_pECS->SystemCamera().DestroyComponent(*m_pComponentCamera);
	}

	if (m_pComponentRender)
	{
		m_pECS->SystemRender().DestroyComponent(*m_pComponentRender);
	}
}

auto JWEntity::CreateComponentTransform() noexcept->SComponentTransform*
{
	return m_pComponentTransform = &m_pECS->SystemTransform().CreateComponent(*this);
}

auto JWEntity::CreateComponentLight() noexcept->SComponentLight*
{
	return m_pComponentLight = &m_pECS->SystemLight().CreateComponent(this);
}

auto JWEntity::CreateComponentPhysics() noexcept->SComponentPhysics*
{
	return m_pComponentPhysics = &m_pECS->SystemPhysics().CreateComponent(this);
}

auto JWEntity::CreateComponentCamera() noexcept->SComponentCamera*
{
	return m_pComponentCamera = &m_pECS->SystemCamera().CreateComponent(this);
}

auto JWEntity::CreateComponentRender() noexcept->SComponentRender*
{
	return m_pComponentRender = &m_pECS->SystemRender().CreateComponent(this);
}
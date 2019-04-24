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

	if (m_pComponentRender)
	{
		m_pECS->SystemRender().DestroyComponent(*m_pComponentRender);
	}

	if (m_pComponentLight)
	{
		m_pECS->SystemLight().DestroyComponent(*m_pComponentLight);
	}

	if (m_pComponentPhysics)
	{
		m_pECS->SystemPhysics().DestroyComponent(*m_pComponentPhysics);
	}
}

auto JWEntity::CreateComponentTransform() noexcept->SComponentTransform*
{
	m_pComponentTransform = &m_pECS->SystemTransform().CreateComponent();
	m_pComponentTransform->PtrEntity = this;

	return m_pComponentTransform;
}

auto JWEntity::CreateComponentRender() noexcept->SComponentRender*
{
	m_pComponentRender = &m_pECS->SystemRender().CreateComponent();
	m_pComponentRender->PtrEntity = this;

	return m_pComponentRender;
}

auto JWEntity::CreateComponentLight() noexcept->SComponentLight*
{
	m_pComponentLight = &m_pECS->SystemLight().CreateComponent();
	m_pComponentLight->PtrEntity = this;

	return m_pComponentLight;
}

auto JWEntity::CreateComponentPhysics() noexcept->SComponentPhysics*
{
	m_pComponentPhysics = &m_pECS->SystemPhysics().CreateComponent();
	m_pComponentPhysics->PtrEntity = this;

	return m_pComponentPhysics;
}

auto JWEntity::CreateComponentCamera() noexcept->SComponentCamera*
{
	m_pComponentCamera = &m_pECS->SystemCamera().CreateComponent();
	m_pComponentCamera->PtrEntity = this;

	return m_pComponentCamera;
}
#include "JWEntity.h"
#include "JWECS.h"

using namespace JWEngine;

JWEntity::~JWEntity()
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
}

auto JWEntity::CreateComponentTransform() noexcept->JWComponentTransform*
{
	m_pComponentTransform = &m_pECS->SystemTransform().CreateComponent();
	m_pComponentTransform->pEntity = this;

	return m_pComponentTransform;
}

auto JWEntity::CreateComponentRender() noexcept->JWComponentRender*
{
	m_pComponentRender = &m_pECS->SystemRender().CreateComponent();
	m_pComponentRender->pEntity = this;

	return m_pComponentRender;
}

auto JWEntity::CreateComponentLight() noexcept->JWComponentLight*
{
	m_pComponentLight = &m_pECS->SystemLight().CreateComponent();
	m_pComponentLight->pEntity = this;

	return m_pComponentLight;
}
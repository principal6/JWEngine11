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
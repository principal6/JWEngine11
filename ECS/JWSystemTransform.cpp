#include "JWSystemTransform.h"

using namespace JWEngine;

void JWSystemTransform::Create(JWECS& ECS) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;
}

void JWSystemTransform::Destroy() noexcept
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}
}

PRIVATE auto JWSystemTransform::CreateComponent(JWEntity& Entity) noexcept->SComponentTransform&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };
	
	auto new_entry{ new SComponentTransform() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID & pointer to Entity
	m_vpComponents[slot]->ComponentID = slot;
	m_vpComponents[slot]->PtrEntity = &Entity;

	return *m_vpComponents[slot];
}

PRIVATE void JWSystemTransform::DestroyComponent(SComponentTransform& Component) noexcept
{
	uint32_t slot{};
	for (const auto& iter : m_vpComponents)
	{
		if (iter->ComponentID == Component.ComponentID)
		{
			break;
		}

		++slot;
	}
	JW_DELETE(m_vpComponents[slot]);

	// Swap the last element of the vector and the deleted element & shrink the size of the vector.
	uint32_t last_index = static_cast<uint32_t>(m_vpComponents.size() - 1);
	if (slot < last_index)
	{
		m_vpComponents[slot] = m_vpComponents[last_index];
		m_vpComponents[slot]->ComponentID = slot; // @important

		m_vpComponents[last_index] = nullptr;
	}

	m_vpComponents.pop_back();
}

void JWSystemTransform::Execute() noexcept
{
	XMMATRIX matrix_translation{};
	XMMATRIX matrix_scaling{};
	XMMATRIX matrix_rotation{};

	for (auto& iter : m_vpComponents)
	{
		if (iter)
		{
			matrix_translation = XMMatrixTranslationFromVector(iter->Position);
			matrix_scaling = XMMatrixScalingFromVector(iter->ScalingFactor);
			matrix_rotation = XMMatrixRotationRollPitchYaw(iter->PitchYawRoll.x, iter->PitchYawRoll.y, iter->PitchYawRoll.z);
			
			switch (iter->WorldMatrixCalculationOrder)
			{
			case JWEngine::EWorldMatrixCalculationOrder::TransRotScale:
				iter->WorldMatrix = matrix_translation * matrix_rotation * matrix_scaling;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::TransScaleRot:
				iter->WorldMatrix = matrix_translation * matrix_scaling * matrix_rotation;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::RotTransScale:
				iter->WorldMatrix = matrix_rotation * matrix_translation * matrix_scaling;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::RotScaleTrans:
				iter->WorldMatrix = matrix_rotation * matrix_scaling * matrix_translation;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::ScaleTransRot:
				iter->WorldMatrix = matrix_scaling * matrix_translation * matrix_rotation;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::ScaleRotTrans:
				iter->WorldMatrix = matrix_scaling * matrix_rotation * matrix_translation;
				break;
			default:
				break;
			}
		}
	}
}
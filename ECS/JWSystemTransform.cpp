#include "JWSystemTransform.h"

using namespace JWEngine;

JWSystemTransform::~JWSystemTransform()
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}
}

auto JWSystemTransform::CreateComponent() noexcept->JWComponentTransform&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };
	
	auto new_entry{ new JWComponentTransform(slot) };
	m_vpComponents.push_back(new_entry);

	return *m_vpComponents[slot];
}

void JWSystemTransform::DestroyComponent(JWComponentTransform& Component) noexcept
{
	uint32_t slot{};
	for (const auto& iter : m_vpComponents)
	{
		if (iter->m_ComponentID == Component.m_ComponentID)
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
		m_vpComponents[last_index] = nullptr;
	}

	m_vpComponents.pop_back();
}

void JWSystemTransform::Update() noexcept
{
	XMMATRIX matrix_translation{};
	XMMATRIX matrix_scaling{};
	XMMATRIX matrix_rotation{};

	for (auto& iter : m_vpComponents)
	{
		if (iter)
		{
			matrix_translation = XMMatrixTranslation(iter->m_Position.x, iter->m_Position.y, iter->m_Position.z);
			matrix_scaling = XMMatrixScaling(iter->m_ScalingFactor.x, iter->m_ScalingFactor.y, iter->m_ScalingFactor.z);
			matrix_rotation = XMMatrixRotationRollPitchYaw(iter->m_Orientation.x, iter->m_Orientation.y, iter->m_Orientation.z);

			switch (iter->m_WorldMatrixCalculationOrder)
			{
			case JWEngine::EWorldMatrixCalculationOrder::TransRotScale:
				iter->m_WorldMatrix = matrix_translation * matrix_rotation * matrix_scaling;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::TransScaleRot:
				iter->m_WorldMatrix = matrix_translation * matrix_scaling * matrix_rotation;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::RotTransScale:
				iter->m_WorldMatrix = matrix_rotation * matrix_translation * matrix_scaling;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::RotScaleTrans:
				iter->m_WorldMatrix = matrix_rotation * matrix_scaling * matrix_translation;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::ScaleTransRot:
				iter->m_WorldMatrix = matrix_scaling * matrix_translation * matrix_rotation;
				break;
			case JWEngine::EWorldMatrixCalculationOrder::ScaleRotTrans:
				iter->m_WorldMatrix = matrix_scaling * matrix_rotation * matrix_translation;
				break;
			default:
				break;
			}
		}
	}
}
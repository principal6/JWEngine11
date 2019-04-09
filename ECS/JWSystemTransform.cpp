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

auto JWSystemTransform::CreateComponent() noexcept->SComponentTransform&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };
	
	auto new_entry{ new SComponentTransform() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID
	m_vpComponents[slot]->ComponentID = slot;

	return *m_vpComponents[slot];
}

void JWSystemTransform::DestroyComponent(SComponentTransform& Component) noexcept
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
			matrix_translation = XMMatrixTranslation(iter->Position.x, iter->Position.y, iter->Position.z);
			matrix_scaling = XMMatrixScaling(iter->ScalingFactor.x, iter->ScalingFactor.y, iter->ScalingFactor.z);
			matrix_rotation = XMMatrixRotationRollPitchYaw(iter->Orientation.x, iter->Orientation.y, iter->Orientation.z);

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
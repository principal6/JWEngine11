#include "JWECS.h"

using namespace JWEngine;

void JWSystemTransform::Create(JWECS& ECS) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;
}

void JWSystemTransform::Destroy() noexcept {}

PRIVATE auto JWSystemTransform::CreateComponent(EntityIndexType EntityIndex) noexcept->ComponentIndexType
{
	auto component_index{ static_cast<ComponentIndexType>(m_vComponents.size()) };
	
	// @important
	// Save component ID & pointer to Entity
	m_vComponents.emplace_back(EntityIndex, component_index);

	return component_index;
}

PRIVATE void JWSystemTransform::DestroyComponent(ComponentIndexType ComponentIndex) noexcept
{
	if (m_vComponents.size() == 0)
	{
		JW_ERROR_ABORT("There is no component to destroy.");
	}

	// Save the target component's index.
	auto component_index{ ComponentIndex };

	// Get the last index of the component vector.
	auto last_index = static_cast<ComponentIndexType>(m_vComponents.size() - 1);

	// Get pointer to the entity with last_index's component.
	auto ptr_entity_last_component = m_pECS->GetEntityByIndex(m_vComponents[last_index].EntityIndex);

	// See if the component index is invalid.
	if (component_index > last_index)
	{
		JW_ERROR_ABORT("Invalid component index.");
	}

	// Swap the last element of the vector and the deleted element if necessary
	if (component_index < last_index)
	{
		m_vComponents[component_index] = std::move(m_vComponents[last_index]);
		m_vComponents[component_index].ComponentIndex = component_index; // @important

		ptr_entity_last_component->SetComponentTransformIndex(component_index); // @important
	}

	// Shrink the size of the vector.
	m_vComponents.pop_back();
}

PRIVATE auto JWSystemTransform::GetComponentPtr(ComponentIndexType ComponentIndex) noexcept->SComponentTransform*
{
	if (ComponentIndex >= m_vComponents.size())
	{
		return nullptr;
	}

	return &m_vComponents[ComponentIndex];
}

void JWSystemTransform::Execute() noexcept
{
	XMMATRIX matrix_translation{};
	XMMATRIX matrix_scaling{};
	XMMATRIX matrix_rotation{};

	for (auto& iter : m_vComponents)
	{
		matrix_translation = XMMatrixTranslationFromVector(iter.Position);
		matrix_scaling = XMMatrixScalingFromVector(iter.ScalingFactor);
		matrix_rotation = XMMatrixRotationRollPitchYaw(iter.PitchYawRoll.x, iter.PitchYawRoll.y, iter.PitchYawRoll.z);

		switch (iter.WorldMatrixCalculationOrder)
		{
		case JWEngine::EWorldMatrixCalculationOrder::TransRotScale:
			iter.WorldMatrix = matrix_translation * matrix_rotation * matrix_scaling;
			break;
		case JWEngine::EWorldMatrixCalculationOrder::TransScaleRot:
			iter.WorldMatrix = matrix_translation * matrix_scaling * matrix_rotation;
			break;
		case JWEngine::EWorldMatrixCalculationOrder::RotTransScale:
			iter.WorldMatrix = matrix_rotation * matrix_translation * matrix_scaling;
			break;
		case JWEngine::EWorldMatrixCalculationOrder::RotScaleTrans:
			iter.WorldMatrix = matrix_rotation * matrix_scaling * matrix_translation;
			break;
		case JWEngine::EWorldMatrixCalculationOrder::ScaleTransRot:
			iter.WorldMatrix = matrix_scaling * matrix_translation * matrix_rotation;
			break;
		case JWEngine::EWorldMatrixCalculationOrder::ScaleRotTrans:
			iter.WorldMatrix = matrix_scaling * matrix_rotation * matrix_translation;
			break;
		default:
			break;
		}
	}
}
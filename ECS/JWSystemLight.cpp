#include "JWECS.h"
#include "../Core/JWDX.h"

using namespace JWEngine;

void JWSystemLight::Create(JWECS& ECS, JWDX& DX) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;

	// Set JWDX pointer.
	m_pDX = &DX;
}

void JWSystemLight::Destroy() noexcept {}

PRIVATE auto JWSystemLight::CreateComponent(EntityIndexType EntityIndex) noexcept->ComponentIndexType
{
	auto component_index{ static_cast<ComponentIndexType>(m_vComponents.size()) };

	// @important
	// Save component ID & pointer to Entity
	m_vComponents.emplace_back(EntityIndex, component_index);

	m_ShouldUpdateLights = true;

	return component_index;
}

PRIVATE void JWSystemLight::DestroyComponent(ComponentIndexType ComponentIndex) noexcept
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

		ptr_entity_last_component->SetComponentLightIndex(component_index); // @important
	}

	// Shrink the size of the vector.
	m_vComponents.pop_back();
}

PRIVATE auto JWSystemLight::GetComponentPtr(ComponentIndexType ComponentIndex) noexcept->SComponentLight*
{
	if (ComponentIndex >= m_vComponents.size())
	{
		return nullptr;
	}

	return &m_vComponents[ComponentIndex];
}

void JWSystemLight::Execute() noexcept
{
	if (m_ShouldUpdateLights)
	{
		for (auto& iter : m_vComponents)
		{
			auto& light_data = iter.LightData;

			if (light_data.LightType == ELightType::AmbientLight)
			{
				m_PSCBLights.AmbientColor.x = light_data.LightColor.x;
				m_PSCBLights.AmbientColor.y = light_data.LightColor.y;
				m_PSCBLights.AmbientColor.z = light_data.LightColor.z;
				m_PSCBLights.AmbientColor.w = light_data.Intensity;
			}
			else if (light_data.LightType == ELightType::DirectionalLight)
			{
				m_PSCBLights.DirectionalColor.x = light_data.LightColor.x;
				m_PSCBLights.DirectionalColor.y = light_data.LightColor.y;
				m_PSCBLights.DirectionalColor.z = light_data.LightColor.z;
				m_PSCBLights.DirectionalColor.w = light_data.Intensity;
				m_PSCBLights.DirectionalDirection.x = light_data.Direction.x;
				m_PSCBLights.DirectionalDirection.y = light_data.Direction.y;
				m_PSCBLights.DirectionalDirection.z = light_data.Direction.z;
			}
		}

		m_pDX->UpdatePSCBLights(m_PSCBLights);

		m_ShouldUpdateLights = false;
	}
}
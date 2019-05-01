#include "JWECS.h"
#include "../Core/JWDX.h"

using namespace JWEngine;

void JWSystemLight::Create(JWDX& DX) noexcept
{
	// Set JWDX pointer.
	m_pDX = &DX;
}

void JWSystemLight::Destroy() noexcept
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}
}

auto JWSystemLight::CreateComponent(JWEntity* pEntity) noexcept->SComponentLight&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };

	auto new_entry{ new SComponentLight() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID & pointer to Entity
	m_vpComponents[slot]->ComponentID = slot;
	m_vpComponents[slot]->PtrEntity = pEntity;

	m_ShouldUpdate = true;

	return *m_vpComponents[slot];
}

void JWSystemLight::DestroyComponent(SComponentLight& Component) noexcept
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

void JWSystemLight::Execute() noexcept
{
	if (m_ShouldUpdate)
	{
		for (auto& iter : m_vpComponents)
		{
			auto& light_data = iter->LightData;

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

		m_ShouldUpdate = false;
	}
}
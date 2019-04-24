#include "JWECS.h"

using namespace JWEngine;

void JWSystemCamera::Create(JWECS& ECS, XMFLOAT2 WindowSize) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;
	
	// Set orthographic projection matrix.
	m_MatrixProjOrthographic = XMMatrixOrthographicLH(WindowSize.x, WindowSize.y, KOrthographicNearZ, KOrthographicFarZ);
}

void JWSystemCamera::Destroy() noexcept
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}
}

auto JWSystemCamera::CreateComponent() noexcept->SComponentCamera&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };

	auto new_entry{ new SComponentCamera() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID
	m_vpComponents[slot]->ComponentID = slot;

	return *m_vpComponents[slot];
}

void JWSystemCamera::DestroyComponent(SComponentCamera& Component) noexcept
{
	if (m_vpComponents.size() == 1)
	{
		JW_ERROR_RETURN("카메라 컴포넌트 해제 실패. 모든 장면에는 최소한 1개의 카메라가 필요합니다.");
	}

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
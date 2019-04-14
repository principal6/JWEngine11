#include "JWSystemPhysics.h"

using namespace JWEngine;

JWSystemPhysics::~JWSystemPhysics()
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}
}

auto JWSystemPhysics::CreateComponent() noexcept->SComponentPhysics&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };

	auto new_entry{ new SComponentPhysics() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID
	m_vpComponents[slot]->ComponentID = slot;

	return *m_vpComponents[slot];
}

void JWSystemPhysics::DestroyComponent(SComponentPhysics& Component) noexcept
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

void JWSystemPhysics::Execute() noexcept
{
	for (auto& iter : m_vpComponents)
	{
		if (iter)
		{
			
		}
	}
}
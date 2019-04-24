#include "JWECS.h"
#include "../Core/JWWin32Window.h"

using namespace JWEngine;

void JWECS::Create(JWDX& DX, JWWin32Window& Window, STRING BaseDirectory) noexcept
{
	m_pDX = &DX;
	m_BaseDirectory = BaseDirectory;

	m_SystemTransform.Create();
	m_SystemRender.Create(*this, DX, BaseDirectory);
	m_SystemLight.Create(DX);
	m_SystemPhysics.Create(*this, Window.GethWnd(), Window.GetWindowSize());
	m_SystemCamera.Create(*this, Window.GetWindowSize());
}

void JWECS::Destroy() noexcept
{
	// Destroy entities and free them
	for (auto& iter : m_vpEntities)
	{
		iter->Destroy();
		JW_DELETE(iter);
	}

	// @important
	// Destroy systems
	m_SystemCamera.Destroy();
	m_SystemPhysics.Destroy();
	m_SystemLight.Destroy();
	m_SystemRender.Destroy();
	m_SystemTransform.Destroy();
}

auto JWECS::CreateEntity(STRING EntityName) noexcept->JWEntity*
{
	if (m_vpEntities.size())
	{
		for (auto& iter : m_vpEntities)
		{
			if (iter->GetEntityName().compare(EntityName) == 0)
			{
				// Duplicate name cannot be used!
				JW_ERROR_ABORT("This entity name is duplicated. (" + EntityName + ")");
			}
		}
	}

	m_vpEntities.push_back(new JWEntity());
	uint64_t index = m_vpEntities.size() - 1;

	auto& entity = m_vpEntities[index];
	entity->Create(this, EntityName);

	m_mapEntityNames.insert(std::make_pair(EntityName, index));

	return entity;
}

auto JWECS::CreateEntity(EEntityType Type) noexcept->JWEntity*
{
	if (Type == EEntityType::UserDefined)
	{
		JW_ERROR_ABORT("You can't make a user-defined entity by this method.");
	}

	if (m_vpEntities.size())
	{
		for (auto& iter : m_vpEntities)
		{
			if (iter->GetEntityType() == Type)
			{
				// It must be unique
				JW_ERROR_ABORT("The entity of the type already exists.");
			}
		}
	}

	STRING temp{};
	STRING entity_name{ "UniqueEntity" };
	uint32_t type_id = static_cast<uint32_t>(Type);
	entity_name += ConvertIntToSTRING(type_id, temp);

	m_vpEntities.push_back(new JWEntity());
	uint64_t index = m_vpEntities.size() - 1;

	auto& entity = m_vpEntities[index];
	entity->Create(this, entity_name, Type);
	m_mapEntityNames.insert(std::make_pair(entity_name, index));

	return entity;
}

auto JWECS::GetEntityByIndex(uint32_t Index) noexcept->JWEntity*
{
	JWEntity* result{};

	if (Index < m_vpEntities.size())
	{
		result = m_vpEntities[Index];
	}

	return result;
}

auto JWECS::GetEntityByName(STRING EntityName) noexcept->JWEntity*
{
	JWEntity* result{};

	if (m_vpEntities.size())
	{
		auto find = m_mapEntityNames.find(EntityName);
		if (find != m_mapEntityNames.end())
		{
			auto index = find->second;

			result = m_vpEntities[index];
		}
		else
		{
			JW_ERROR_ABORT("Unable to find the entity of name (" + EntityName + ")");
		}
	}

	return result;
}

auto JWECS::GetEntityByType(EEntityType Type) noexcept->JWEntity*
{
	JWEntity* result{};

	if (Type == EEntityType::UserDefined)
	{
		JW_ERROR_ABORT("Impossible to get the entity of user defined type by calling this method.");
	}

	if (m_vpEntities.size())
	{
		for (auto& iter : m_vpEntities)
		{
			if (iter->GetEntityType() == Type)
			{
				result = iter;
			}
		}
	}

	return result;
}

void JWECS::DestroyEntityByIndex(uint32_t Index) noexcept
{
	if (Index < m_vpEntities.size())
	{
		JW_DELETE(m_vpEntities[Index]);

		uint32_t last_Index = static_cast<uint32_t>(m_vpEntities.size() - 1);
		if (Index < last_Index)
		{
			m_vpEntities[Index] = m_vpEntities[last_Index];
			m_vpEntities[last_Index] = nullptr;
		}

		m_vpEntities.pop_back();
	}
}

void JWECS::DestroyEntityByName(STRING EntityName) noexcept
{
	if (m_vpEntities.size())
	{
		auto find = m_mapEntityNames.find(EntityName);
		if (find != m_mapEntityNames.end())
		{
			auto index = find->second;

			JW_DELETE(m_vpEntities[index]);

			uint32_t last_index = static_cast<uint32_t>(m_vpEntities.size() - 1);
			if (index < last_index)
			{
				m_vpEntities[index] = m_vpEntities[last_index];
				m_vpEntities[last_index] = nullptr;
			}

			m_vpEntities.pop_back();
		}
		else
		{
			JW_ERROR_ABORT("Unable to fine the entity of name (" + EntityName + ")");
		}
	}
}

void JWECS::DestroyEntityByType(EEntityType Type) noexcept
{
	if (Type == EEntityType::UserDefined)
	{
		JW_ERROR_ABORT("Impossible to destroy the entity of user defined type by calling this method.");
	}

	if (m_vpEntities.size())
	{
		uint32_t index{};
		for (auto& iter : m_vpEntities)
		{
			if (iter->GetEntityType() == Type)
			{
				JW_DELETE(m_vpEntities[index]);

				uint32_t last_index = static_cast<uint32_t>(m_vpEntities.size() - 1);
				if (index < last_index)
				{
					m_vpEntities[index] = m_vpEntities[last_index];
					m_vpEntities[last_index] = nullptr;
				}

				m_vpEntities.pop_back();

				return;
			}

			++index;
		}
	}
}

void JWECS::ExecuteSystems() noexcept
{
	m_SystemTransform.Execute();

	m_SystemLight.Execute();

	m_SystemRender.Execute();

	m_SystemPhysics.Execute();
}
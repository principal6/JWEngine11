#include "JWECS.h"

using namespace JWEngine;

void JWECS::Create(JWDX& DX, HWND hWnd, const SSize2& WindowSize, STRING BaseDirectory) noexcept
{
	m_pDX = &DX;
	m_BaseDirectory = BaseDirectory;

	m_SystemTransform.Create(*this);
	m_SystemLight.Create(*this, DX);
	m_SystemPhysics.Create(*this, hWnd, WindowSize);
	m_SystemCamera.Create(*this, DX, WindowSize);
	m_SystemRender.Create(*this, DX, WindowSize, BaseDirectory);
}

void JWECS::Destroy() noexcept
{
	// Destroy entities and free them
	for (auto& iter : m_vEntities)
	{
		iter.Destroy();
	}

	// @important
	// Destroy systems
	m_SystemRender.Destroy();
	m_SystemCamera.Destroy();
	m_SystemPhysics.Destroy();
	m_SystemLight.Destroy();
	m_SystemTransform.Destroy();
}

auto JWECS::CreateEntity(STRING EntityName) noexcept->JWEntity*
{
	if (m_vEntities.size())
	{
		for (auto& iter : m_vEntities)
		{
			if (iter.GetEntityName().compare(EntityName) == 0)
			{
				// Duplicate name cannot be used!
				JW_ERROR_ABORT("This entity name is duplicated. (" + EntityName + ")");
			}
		}
	}

	auto entity_index{ static_cast<EntityIndexType>(m_vEntities.size()) };
	m_vEntities.emplace_back(entity_index);

	auto& entity = m_vEntities[entity_index];
	entity.Create(this, EntityName);

	m_mapEntityNames.insert(std::make_pair(EntityName, entity_index));

	return &entity;
}

auto JWECS::CreateEntity(EEntityType Type) noexcept->JWEntity*
{
	if (Type == EEntityType::UserDefined)
	{
		JW_ERROR_ABORT("You can't make a user-defined entity by this method.");
	}

	if (m_vEntities.size())
	{
		for (auto& iter : m_vEntities)
		{
			if (iter.GetEntityType() == Type)
			{
				// It must be unique
				JW_ERROR_ABORT("The entity of the type already exists.");
			}
		}
	}

	STRING entity_name{ "UniqueEntity" };
	uint32_t type_id = static_cast<uint32_t>(Type);
	entity_name += TO_STRING(type_id);

	auto entity_index{ static_cast<EntityIndexType>(m_vEntities.size()) };
	m_vEntities.emplace_back(entity_index);

	auto& entity = m_vEntities[entity_index];
	entity.Create(this, entity_name, Type);
	m_mapEntityNames.insert(std::make_pair(entity_name, entity_index));

	return &entity;
}

auto JWECS::GetEntityByIndex(EntityIndexType Index) noexcept->JWEntity*
{
	JWEntity* result{};

	if (Index < m_vEntities.size())
	{
		result = &m_vEntities[Index];
	}
	else
	{
		JW_ERROR_ABORT("Invalid entity index.");
	}

	return result;
}

auto JWECS::GetEntityByName(const STRING& EntityName) noexcept->JWEntity*
{
	JWEntity* result{};

	if (m_vEntities.size())
	{
		auto find = m_mapEntityNames.find(EntityName);
		if (find != m_mapEntityNames.end())
		{
			auto entity_index = find->second;

			result = &m_vEntities[entity_index];
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

	if (m_vEntities.size())
	{
		for (auto& iter : m_vEntities)
		{
			if (iter.GetEntityType() == Type)
			{
				result = &iter;
			}
		}
	}

	return result;
}

void JWECS::DestroyEntityByIndex(EntityIndexType Index) noexcept
{
	if (Index < m_vEntities.size())
	{
		m_mapEntityNames.erase(m_vEntities[Index].GetEntityName());

		m_vEntities[Index].Destroy();

		auto last_Index = static_cast<EntityIndexType>(m_vEntities.size() - 1);
		if (Index < last_Index)
		{
			m_vEntities[Index] = std::move(m_vEntities[last_Index]);
			m_vEntities[Index].SetEntityIndex(Index);
		}

		m_vEntities.pop_back();
	}
	else
	{
		JW_ERROR_ABORT("Invalid entity index.");
	}
}

void JWECS::DestroyEntityByName(const STRING& EntityName) noexcept
{
	if (m_vEntities.size())
	{
		auto find = m_mapEntityNames.find(EntityName);
		if (find != m_mapEntityNames.end())
		{
			auto entity_index = find->second;

			DestroyEntityByIndex(entity_index);
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

	if (m_vEntities.size())
	{
		for (auto& iter : m_vEntities)
		{
			if (iter.GetEntityType() == Type)
			{
				auto entity_index = iter.GetEntityIndex();

				DestroyEntityByIndex(entity_index);

				return;
			}
		}
	}
}

void JWECS::UpdateDeltaTime(long long dt) noexcept
{
	m_DeltaTime = static_cast<float>(dt) / 1'000'000.0f;
}

auto JWECS::GetDeltaTime() noexcept->float
{
	return m_DeltaTime;
}

void JWECS::ExecuteSystems() noexcept
{
	m_SystemTransform.Execute();

	// Camera must be executed before Light and Phyiscs
	m_SystemCamera.Execute();

	m_SystemLight.Execute();

	m_SystemPhysics.Execute();

	// Render must be the last one to be executed
	m_SystemRender.Execute();
}
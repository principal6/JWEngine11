#pragma once

#include "JWEntity.h"

namespace JWEngine
{
	class JWDX;

	class JWECS final
	{
	public:
		JWECS() = default;
		~JWECS() = default;

		// Called in JWGame class
		void Create(JWDX& DX, HWND hWnd, const SSize2& WindowSize, STRING BaseDirectory) noexcept;
		void Destroy() noexcept;
		
		// ### Entity creator ###
		// Creates non-unique entity without specifying the type (user-defined type)
		auto CreateEntity(STRING EntityName) noexcept->JWEntity*;
		
		// Creates unique entity
		auto CreateEntity(EEntityType Type) noexcept->JWEntity*;

		// ### Entity getters ###
		auto GetEntityByIndex(EntityIndexType Index) noexcept->JWEntity*;
		auto GetEntityByName(const STRING& EntityName) noexcept->JWEntity*;
		auto GetEntityByType(EEntityType Type) noexcept->JWEntity*;

		// ### Entity destroyers ###
		void DestroyEntityByIndex(EntityIndexType Index) noexcept;
		void DestroyEntityByName(const STRING& EntityName) noexcept;
		void DestroyEntityByType(EEntityType Type) noexcept;

		void UpdateDeltaTime(long long dt) noexcept;
		auto GetDeltaTime() noexcept->float;

		// ### Execute ###
		void ExecuteSystems() noexcept;

		// ### Object getters ###
		auto& SystemTransform() noexcept { return m_SystemTransform; }
		auto& SystemLight() noexcept { return m_SystemLight; }
		auto& SystemPhysics() noexcept { return m_SystemPhysics; }
		auto& SystemCamera() noexcept { return m_SystemCamera; }
		auto& SystemRender() noexcept { return m_SystemRender; }

	private:
		JWDX*					m_pDX{};
		STRING					m_BaseDirectory{};

		JWSystemTransform		m_SystemTransform{};
		JWSystemLight			m_SystemLight{};
		JWSystemPhysics			m_SystemPhysics{};
		JWSystemCamera			m_SystemCamera{};
		JWSystemRender			m_SystemRender{};

		VECTOR<JWEntity>				m_vEntities;
		MAP<STRING, EntityIndexType>	m_mapEntityNames;

		float					m_DeltaTime{};
	};
};


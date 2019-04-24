#pragma once

#include "JWEntity.h"

namespace JWEngine
{
	class JWDX;
	class JWWin32Window;

	class JWECS final
	{
	public:
		JWECS() = default;
		~JWECS() = default;

		// Called in JWGame class
		void Create(JWDX& DX, JWWin32Window& Window, STRING BaseDirectory) noexcept;
		void Destroy() noexcept;
		
		// ### Entity creator ###
		// Creates non-unique entity without specifying the type (user-defined type)
		auto CreateEntity(STRING EntityName) noexcept->JWEntity*;
		
		// Creates unique entity
		auto CreateEntity(EEntityType Type) noexcept->JWEntity*;

		// ### Entity getters ###
		auto GetEntityByIndex(uint32_t Index) noexcept->JWEntity*;
		auto GetEntityByName(STRING EntityName) noexcept->JWEntity*;
		auto GetEntityByType(EEntityType Type) noexcept->JWEntity*;

		// ### Entity destroyers ###
		void DestroyEntityByIndex(uint32_t Index) noexcept;
		void DestroyEntityByName(STRING EntityName) noexcept;
		void DestroyEntityByType(EEntityType Type) noexcept;

		// ### Execute ###
		void ExecuteSystems() noexcept;

		// ### Object getters ###
		auto& SystemTransform() noexcept { return m_SystemTransform; }
		auto& SystemRender() noexcept { return m_SystemRender; }
		auto& SystemLight() noexcept { return m_SystemLight; }
		auto& SystemPhysics() noexcept { return m_SystemPhysics; }
		auto& SystemCamera() noexcept { return m_SystemCamera; }

	private:
		JWDX*					m_pDX{};
		JWWin32Window*			m_pWindow{};
		STRING					m_BaseDirectory{};

		JWSystemTransform		m_SystemTransform{};
		JWSystemRender			m_SystemRender{};
		JWSystemLight			m_SystemLight{};
		JWSystemPhysics			m_SystemPhysics{};
		JWSystemCamera			m_SystemCamera{};

		VECTOR<JWEntity*>		m_vpEntities;
		MAP<STRING, uint64_t>	m_mapEntityNames;
	};
};


#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;

	struct SComponentPhysics
	{
		JWEntity*	PtrEntity{};
		uint32_t	ComponentID{};

		float		BoundingSphereRadious{ 1.0f };
	};

	class JWSystemPhysics
	{
	public:
		JWSystemPhysics() {};
		~JWSystemPhysics();

		auto CreateComponent() noexcept->SComponentPhysics &;
		void DestroyComponent(SComponentPhysics& Component) noexcept;

		void Execute() noexcept;

	private:
		VECTOR<SComponentPhysics*>	m_vpComponents;
	};
};

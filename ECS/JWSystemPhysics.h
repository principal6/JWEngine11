#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;
	class JWECS;

	struct SComponentPhysics
	{
		JWEntity*			PtrEntity{};
		uint32_t			ComponentID{};

		// Unique bounding ellipsoid that covers the whole entity.
		SBoundingEllipsoidData			BoundingEllipsoid{};

		// Subset of bounding ellipsoids that cover parts of the entity.
		VECTOR<SBoundingEllipsoidData>	SubBoundingEllipsoids{};
	};

	class JWSystemPhysics
	{
	public:
		JWSystemPhysics() = default;
		~JWSystemPhysics() = default;

		void Create(JWECS& ECS, HWND hWnd, const SSize2& WindowSize) noexcept;
		void Destroy() noexcept;

		auto CreateComponent(JWEntity* pEntity) noexcept->SComponentPhysics&;
		void DestroyComponent(SComponentPhysics& Component) noexcept;

		auto PickEntity() noexcept->bool;
		auto PickSubBoundingEllipsoid() noexcept->bool;
		void PickTerrainTriangle() noexcept;

		auto IsEntityPicked() const noexcept { return m_IsEntityPicked; };
		const auto GetPickedEntity() const noexcept { return m_pPickedEntity; };
		const auto& GetPickedEntityName() const noexcept { return m_PickedEntityName; };

		const auto& GetPickingRayOrigin() const noexcept { return m_PickingRayOrigin; };
		const auto& GetPickingRayDirection() const noexcept { return m_PickingRayDirection; };
		
		// Only available when Triangle picking is done.
		const auto& GetPickedTriangleVertex(uint32_t PositionIndex) const noexcept { return m_PickedTriangle[min(PositionIndex, 2)]; };

		// Only available when Triangle picking is done.
		const auto& GetPickedPoint() const noexcept { return m_PickedPoint; };

		void Execute() noexcept;

	private:
		__forceinline void CastPickingRay() noexcept;

		// Picking #0 Triangle picking (experimental)
		auto PickEntityByTriangle() noexcept->bool;

		// Picking #1 Ellipsoid picking (realistic choice)
		auto PickEntityByEllipsoid() noexcept->bool;

		void UpdateBoundingEllipsoid(SComponentPhysics& Physics) noexcept;

	private:
		VECTOR<SComponentPhysics*>	m_vpComponents;

		JWECS*						m_pECS{};

		HWND						m_hWnd{};
		const SSize2*				m_pWindowSize{};

		POINT						m_MouseClientPosition{};
		XMFLOAT2					m_NormalizedMousePosition{};
		XMVECTOR					m_PickingRayViewSpacePosition{ XMVectorSet(0, 0, 0, 0) };
		XMVECTOR					m_PickingRayViewSpaceDirection{};
		XMVECTOR					m_PickingRayOrigin{};
		XMVECTOR					m_PickingRayDirection{};
		XMVECTOR					m_PickedTriangle[3]{};
		XMVECTOR					m_PickedPoint{};
		VECTOR<uint32_t>			m_vPickedSubBoundingEllipsoidID{};

		bool						m_IsEntityPicked{ false };
		STRING						m_PickedEntityName{};
		JWEntity*					m_pPickedEntity{};
		
	};
};

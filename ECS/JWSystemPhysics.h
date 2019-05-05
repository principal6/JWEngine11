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

		// Unique bounding sphere that covers the whole entity
		SBoundingSphereData			BoundingSphereData{};

		// Subset of bounding spheres that cover a part of the entity
		VECTOR<SBoundingSphereData>	SubBoundingSpheres{};
	};

	class JWSystemPhysics
	{
	public:
		JWSystemPhysics() = default;
		~JWSystemPhysics() = default;

		void Create(JWECS& ECS, HWND hWnd, XMFLOAT2 WindowSize) noexcept;
		void Destroy() noexcept;

		auto CreateComponent(JWEntity* pEntity) noexcept->SComponentPhysics&;
		void DestroyComponent(SComponentPhysics& Component) noexcept;

		// Component setting
		void SetBoundingSphere(JWEntity* pEntity, float Radius, const XMFLOAT3& CenterOffset = XMFLOAT3(0, 0, 0)) noexcept;
		void HideBoundingSphere(JWEntity* pEntity) noexcept;
		void UnhideBoundingSphere(JWEntity* pEntity) noexcept;
		void UpdateBoundingSphere(JWEntity* pEntity) noexcept;
		void SetSubBoundingSpheres(JWEntity* pEntity, const VECTOR<SBoundingSphereData>& vData) noexcept;

		auto Pick() noexcept->bool;
		auto PickSubBoundingSphere() noexcept->bool;
		void PickTerrainTriangle() noexcept;

		const auto GetPickedEntity() const noexcept { return m_pPickedEntity; };

		const auto& GetPickingRayOrigin() const noexcept { return m_PickingRayOrigin; };
		const auto& GetPickingRayDirection() const noexcept { return m_PickingRayDirection; };
		const auto& GetPickedEntityName() const noexcept { return m_PickedEntityName; };
		
		// Only available when Triangle picking is done.
		const auto& GetPickedTriangleVertex(uint32_t PositionIndex) const noexcept { return m_PickedTriangle[min(PositionIndex, 2)]; };

		// Only available when Triangle picking is done.
		const auto& GetPickedPoint() const noexcept { return m_PickedPoint; };

		void Execute() noexcept;

	private:
		__forceinline void CastPickingRay() noexcept;

		// Picking #0 Triangle picking (experimental)
		auto PickEntityByTriangle() noexcept->bool;

		// Picking #1 Sphere picking (realistic choice)
		auto PickEntityBySphere() noexcept->bool;

	private:
		VECTOR<SComponentPhysics*>	m_vpComponents;

		JWECS*						m_pECS{};

		HWND						m_hWnd{};
		XMFLOAT2					m_WindowSize{};

		POINT						m_MouseClientPosition{};
		XMFLOAT2					m_NormalizedMousePosition{};
		XMVECTOR					m_PickingRayViewSpacePosition{ XMVectorSet(0, 0, 0, 0) };
		XMVECTOR					m_PickingRayViewSpaceDirection{};
		XMVECTOR					m_PickingRayOrigin{};
		XMVECTOR					m_PickingRayDirection{};
		XMVECTOR					m_PickedTriangle[3]{};
		XMVECTOR					m_PickedPoint{};
		STRING						m_PickedEntityName{};
		JWEntity*					m_pPickedEntity{};
		int32_t						m_PickedSubBSID{ -1 };
	};
};

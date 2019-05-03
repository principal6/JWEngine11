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

		auto GetPickingRayOrigin() const noexcept->const XMVECTOR&;
		auto GetPickingRayDirection() const noexcept->const XMVECTOR&;
		
		auto GetPickedEntityName() const noexcept->const STRING&;
		auto GetPickedEntity() const noexcept { return m_pPickedEntity; }

		// Only available when Triangle picking is done.
		auto GetPickedTrianglePosition(uint32_t PositionIndex) const noexcept->const XMVECTOR&;

		// Only available when Triangle picking is done.
		auto& GetPickedPointPosition() const noexcept { return m_PickedPoint; };

		void Execute() noexcept;

	private:
		void CastPickingRay() noexcept;

		// Picking #0 Triangle picking (experimental)
		auto PickEntityByTriangle() noexcept->bool;
		__forceinline auto PickTriangle(const XMVECTOR& V0, const XMVECTOR& V1, const XMVECTOR& V2, XMVECTOR& t_cmp) noexcept->bool;
		__forceinline auto IsPointInTriangle(const XMVECTOR& Point, const XMVECTOR& V0, const XMVECTOR& V1, const XMVECTOR& V2) noexcept->bool;

		// Picking #1 Sphere picking (realistic choice)
		auto PickEntityBySphere() noexcept->bool;
		__forceinline auto IsSpherePicked(const XMVECTOR& Center, float Radius) noexcept->bool;

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

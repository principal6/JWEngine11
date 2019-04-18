#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWWin32Window;
	class JWCamera;
	class JWEntity;
	class JWECS;

	struct SComponentPhysics
	{
		JWEntity*	PtrEntity{};
		uint32_t	ComponentID{};

		float		BoundingSphereRadius{};
		XMFLOAT3	BoundingSphereCenter{};
	};

	class JWSystemPhysics
	{
	public:
		JWSystemPhysics() = default;
		~JWSystemPhysics() = default;

		void Create(JWECS& ECS, const JWWin32Window& Window, const JWCamera& Camera) noexcept;
		void Destroy() noexcept;

		auto CreateComponent() noexcept->SComponentPhysics&;
		void DestroyComponent(SComponentPhysics& Component) noexcept;

		/// Component setting
		void CreateBoundingSphere(SComponentPhysics* pComponent, float Radius, const XMFLOAT3& CenterOffset = XMFLOAT3(0, 0, 0)) noexcept;

		void Pick() noexcept;

		auto GetPickingRayOrigin() const noexcept->const XMVECTOR&;
		auto GetPickingRayDirection() const noexcept->const XMVECTOR&;
		
		auto GetPickedEntityName() const noexcept->const STRING &;

		// Only available when Triangle picking is chosen. (experimental)
		auto GetPickedTrianglePosition(uint32_t PositionIndex) const noexcept->const XMVECTOR&;

		void Execute() noexcept;

	private:
		void CastPickingRay() noexcept;

		// Picking #0 Triangle picking (experimental)
		void PickEntityByTriangle() noexcept;
		__forceinline auto PickTriangle(XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2, XMVECTOR& t_cmp) noexcept->bool;
		__forceinline auto IsPointInTriangle(XMVECTOR& Point, XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2) noexcept->bool;

		// Picking #1 Sphere picking (realistic choice)
		void PickEntityBySphere() noexcept;

	private:
		VECTOR<SComponentPhysics*>	m_vpComponents;

		JWECS*						m_pECS{};
		const JWWin32Window*		m_pWindow{};
		const JWCamera*				m_pCamera{};

		HWND						m_hWnd{};
		float						m_WindowWidth{};
		float						m_WindowHeight{};

		POINT						m_MouseClientPosition{};
		XMFLOAT2					m_NormalizedMousePosition{};
		XMVECTOR					m_PickingRayViewSpacePosition{ XMVectorSet(0, 0, 0, 0) };
		XMVECTOR					m_PickingRayViewSpaceDirection{};
		XMVECTOR					m_PickingRayOrigin{};
		XMVECTOR					m_PickingRayDirection{};
		XMVECTOR					m_PickedTriangle[3]{};
		STRING						m_PickedEntityName{};
		JWEntity*					m_pPickedEntity{};
	};
};

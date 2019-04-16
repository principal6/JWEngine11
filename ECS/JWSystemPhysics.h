#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWWin32Window;
	class JWCamera;
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
		JWSystemPhysics() = default;
		~JWSystemPhysics() = default;

		void Create(JWWin32Window& Window, JWCamera& Camera) noexcept;
		void Destroy() noexcept;

		auto CreateComponent() noexcept->SComponentPhysics&;
		void DestroyComponent(SComponentPhysics& Component) noexcept;

		void Pick() noexcept;

		auto GetPickingRayOrigin() const noexcept->const XMVECTOR&;
		auto GetPickingRayDirection() const noexcept->const XMVECTOR&;
		auto GetPickedTrianglePosition(uint32_t PositionIndex) const noexcept->const XMVECTOR&;

		void Execute() noexcept;

	private:
		void CastPickingRay() noexcept;
		void PickEntityByTriangle() noexcept;
		__forceinline auto PickTriangle(XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2, XMVECTOR& t_cmp) noexcept->XMVECTOR;
		__forceinline auto IsPointInTriangle(XMVECTOR& Point, XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2) noexcept->bool;

	private:
		VECTOR<SComponentPhysics*>	m_vpComponents;

		JWWin32Window*			m_pWindow{};
		JWCamera*				m_pCamera{};

		HWND					m_hWnd{};
		float					m_WindowWidth{};
		float					m_WindowHeight{};

		POINT					m_MouseClientPosition{};
		XMFLOAT2				m_NormalizedMousePosition{};
		XMVECTOR				m_PickingRayViewSpacePosition{ XMVectorSet(0, 0, 0, 0) };
		XMVECTOR				m_PickingRayViewSpaceDirection{};
		XMVECTOR				m_PickingRayOrigin{};
		XMVECTOR				m_PickingRayDirection{};
		XMVECTOR				m_PickedTriangle[3]{};
	};
};

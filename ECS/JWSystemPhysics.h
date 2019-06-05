#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	// Gravity on Earth = 9.8m/s^2
	static constexpr float		KGravityOnEarth{ 9.8f };
	static constexpr float		KPhysicsWorldFloor{ -100.0f };
	static constexpr XMVECTOR	KVectorGravityOnEarth{ 0, -KGravityOnEarth, 0, 0 };
	static const STRING			KNoName{ "" };

	class JWEntity;
	class JWECS;

	enum EFLAGSystemPhysicsOption : uint16_t
	{
		JWFlagSystemPhysicsOption_ApplyForces = 0x01,
	};
	using JWFlagSystemPhysicsOption = uint16_t;

	struct SComponentPhysics
	{
		SComponentPhysics(EntityIndexType _EntityIndex, ComponentIndexType _ComponentIndex) :
			EntityIndex{ _EntityIndex }, ComponentIndex{ _ComponentIndex } {};

		EntityIndexType		EntityIndex{};
		ComponentIndexType	ComponentIndex{};

		// Unique bounding ellipsoid that covers the whole entity.
		SBoundingEllipsoidData			BoundingEllipsoid{};

		// Subset of bounding ellipsoids that cover parts of the entity.
		VECTOR<SBoundingEllipsoidData>	SubBoundingEllipsoids{};

		// [Property]	mass (*inverse)
		// [Unit]		gram
		// InverseMass = 1/Mass
		// @important: mass MUST NOT be 'zero'
		// but 'infinite mass' (zero InverseMass) means no physics calculation on this component.
		float		InverseMass{};

		// [Property]	velocity
		// [Unit]		m/s
		XMVECTOR	Velocity{};

		// [Property]	acceleration
		// [Unit]		m/s^2
		XMVECTOR	Acceleration{};

		// @important: AccumulatedForce must be zeroed every frame.
		XMVECTOR	AccumulatedForce{};

		void SetMassByGram(float g)
		{
			assert(g > 0);

			if (g > 0)
			{
				InverseMass = 1.0f / g;
			}
		}

		void SetMassByKilogram(float Kg)
		{
			assert(Kg > 0);

			if (Kg > 0)
			{
				InverseMass = 1.0f / (Kg / 1000.0f);
			}
		}
		
		void SetMassToInfinite()
		{
			InverseMass = 0;
		}

		void SetVelocity(const XMVECTOR& _Velocity)
		{
			Velocity = _Velocity;
		}

		void SetAcceleration(const XMVECTOR& _Acceleration)
		{
			Acceleration = _Acceleration;
		}

		void AddForce(const XMVECTOR& Force)
		{
			AccumulatedForce += Force;
		}
	};

	class JWSystemPhysics
	{
		friend class JWEntity;

	public:
		JWSystemPhysics() = default;
		~JWSystemPhysics() = default;

		void Create(JWECS& ECS, HWND hWnd, const SSize2& WindowSize) noexcept;
		void Destroy() noexcept;

		auto PickEntity() noexcept->bool;

		void SetSystemPhysicsFlag(JWFlagSystemPhysicsOption Flag) noexcept;
		void ToggleSystemPhysicsFlag(JWFlagSystemPhysicsOption Flag) noexcept;

		const auto GetPickedEntity() const noexcept { return m_pPickedEntity; };
		auto GetPickedEntityName() const noexcept->const STRING&;

		const auto& GetPickingRayOrigin() const noexcept { return m_PickingRayOrigin; };
		const auto& GetPickingRayDirection() const noexcept { return m_PickingRayDirection; };

		const auto& GetPickedTriangleVertex(uint32_t PositionIndex) const noexcept { return m_PickedTriangle[min(PositionIndex, 2)]; };
		const auto& GetPickedPoint() const noexcept { return m_PickedPoint; };

		void ApplyUniversalGravity() noexcept;
		void ApplyUniversalAcceleration(const XMVECTOR& _Acceleration) noexcept;
		void ZeroAllVelocities() noexcept;
		void Execute() noexcept;

	// Only accesible for JWEntity
	private:
		auto CreateComponent(EntityIndexType EntityIndex) noexcept->ComponentIndexType;
		void DestroyComponent(ComponentIndexType ComponentIndex) noexcept;
		auto GetComponentPtr(ComponentIndexType ComponentIndex) noexcept->SComponentPhysics*;

	private:
		// Picking
		__forceinline void CastPickingRay() noexcept;
		auto PickEntityByEllipsoid() noexcept->bool;
		auto PickSubBoundingEllipsoid(JWEntity* PtrEntity) noexcept->bool;
		void PickTerrainTriangle() noexcept;

		void UpdateBoundingEllipsoid(SComponentPhysics& Physics) noexcept;
		void UpdateSubBoundingEllipsoids(SComponentPhysics& Physics) noexcept;

	private:
		VECTOR<SComponentPhysics>	m_vComponents;

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
		JWEntity*					m_pPickedEntity{};
		JWEntity*					m_pPickedTerrainEntity{};
		JWEntity*					m_pPickedNonTerrainEntity{};
		XMVECTOR					m_PickedDistance{};
		XMVECTOR					m_PickedTerrainDistance{};
		XMVECTOR					m_PickedNonTerrainDistance{};

		JWFlagSystemPhysicsOption	m_FlagSystemPhyscisOption{};
	};

	class JWPhysicsCollisionDetector
	{
	public:


	private:

	};
};

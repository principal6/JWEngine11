#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	// Gravity on Earth = 9.8m/s^2
	static constexpr float		KNonPhysicalObjectInverseMass{ -1.0f };
	static constexpr float		KGravityOnEarth{ 0.8f };
	static constexpr float		KPhysicsWorldFloor{ -100.0f };
	static constexpr XMVECTOR	KVectorGravityOnEarth{ 0, -KGravityOnEarth, 0, 0 };
	static const STRING			KNoName{ "" };

	class JWEntity;
	class JWECS;

	enum EFLAGSystemPhysicsOption : uint8_t
	{
		JWFlagSystemPhysicsOption_ApplyForces = 0x01,
	};
	using JWFlagSystemPhysicsOption = uint8_t;

	struct SComponentPhysics
	{
		SComponentPhysics(EntityIndexType _EntityIndex, ComponentIndexType _ComponentIndex) :
			EntityIndex{ _EntityIndex }, ComponentIndex{ _ComponentIndex } {};

		EntityIndexType		EntityIndex{};
		ComponentIndexType	ComponentIndex{};

		/// Unique bounding ellipsoid that covers the whole entity.
		///SBoundingEllipsoidData			BoundingEllipsoid{};

		/// Subset of bounding ellipsoids that cover parts of the entity.
		///VECTOR<SBoundingEllipsoidData>	SubBoundingEllipsoids{};

		// Unique bounding sphere that covers the whole entity.
		SBoundingSphereData			BoundingSphere{};

		// Subset of bounding spheres that cover parts of the entity.
		VECTOR<SBoundingSphereData>	SubBoundingSpheres{};

		// [Property]	mass (*inverse)
		// [Unit]		gram
		// InverseMass = 1/Mass
		// @important: mass MUST NOT be 'zero'
		// but 'infinite mass' (zero InverseMass) means no physics calculation on this component.
		float		InverseMass{ KNonPhysicalObjectInverseMass };

		// [Property]	velocity
		// [Unit]		m/s
		XMVECTOR	Velocity{};

		// [Property]	acceleration
		// [Unit]		m/s^2
		XMVECTOR	Acceleration{};

		// @important: AccumulatedForce must be zeroed every frame.
		XMVECTOR	AccumulatedForce{};

		// (NON_OWNING) Collision mesh
		JWModel*	PtrCollisionMesh{};

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
			SetMassByGram(Kg * 1000.0f);
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

		void SetCollisionMesh(JWModel* PtrModel)
		{
			assert(PtrModel);
			PtrCollisionMesh = PtrModel;
		}
	};

	struct SCollisionPair
	{
		SCollisionPair() {};
		SCollisionPair(ComponentIndexType _A, ComponentIndexType _B) : A{ _A }, B{ _B } {};

		ComponentIndexType A{};
		ComponentIndexType B{};
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
		//auto PickEntityByEllipsoid() noexcept->bool;
		auto PickEntityBySphere() noexcept->bool;
		//auto PickSubBoundingEllipsoid(JWEntity* PtrEntity) noexcept->bool;
		auto PickSubBoundingSphere(JWEntity* PtrEntity) noexcept->bool;
		void PickTerrainTriangle() noexcept;

		///void UpdateBoundingEllipsoid(SComponentPhysics& Physics) noexcept;
		///void UpdateSubBoundingEllipsoids(SComponentPhysics& Physics) noexcept;

		void UpdateBoundingSphere(SComponentPhysics& Physics) noexcept;
		void UpdateSubBoundingSpheres(SComponentPhysics& Physics) noexcept;

		void DetectCoarseCollision() noexcept;
		void DetectFineCollision() noexcept;

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
		///VECTOR<uint32_t>			m_vPickedSubBoundingEllipsoidID{};
		VECTOR<uint32_t>			m_vPickedSubBoundingSphereID{};

		JWEntity*					m_pPickedEntity{};
		JWEntity*					m_pPickedTerrainEntity{};
		JWEntity*					m_pPickedNonTerrainEntity{};
		XMVECTOR					m_PickedDistance{};
		XMVECTOR					m_PickedTerrainDistance{};
		XMVECTOR					m_PickedNonTerrainDistance{};

		VECTOR<SCollisionPair>		m_CoarseCollisionList{};

		JWFlagSystemPhysicsOption	m_FlagSystemPhyscisOption{};
	};
};

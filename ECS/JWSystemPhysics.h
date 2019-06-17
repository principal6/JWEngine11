#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	// Gravity on Earth = 9.8m/s^2
	static constexpr float		KNonPhysicalObjectInverseMass{ -1.0f };
	static constexpr float		KGravityOnEarth{ 0.8f };
	static constexpr float		KPhysicsWorldFloor{ -100.0f };
	static constexpr XMVECTOR	KVectorGravityOnEarth{ 0, -KGravityOnEarth, 0, 0 };
	//static constexpr XMVECTOR	KVectorGravityOnEarth{ KGravityOnEarth, 0, 0, 0 };
	static const STRING			KNoName{ "" };

	class JWEntity;
	class JWECS;
	
	enum class ECollisionType
	{
		None,
		PointAFaceB,
		PointBFaceA,
		EdgeEdge,
	};
	
	enum class EClosestEdgePair
	{
		None,
		V0V1,
		V0V2,
		V1V2,
	};

	struct SClosestPoint
	{
		SClosestPoint() {};
		SClosestPoint(size_t _PositionIndex, float _Distance) : PositionIndex{ _PositionIndex }, Distance{ _Distance } {};

		size_t	PositionIndex{};
		float	Distance{};
	};
	
	struct SClosestFace
	{
		SClosestFace() {};
		SClosestFace(const XMVECTOR& _V0, const XMVECTOR& _V1, const XMVECTOR& _V2) : V0{ _V0 }, V1{ _V1 }, V2{ _V2 } {};

		XMVECTOR V0{};
		XMVECTOR V1{};
		XMVECTOR V2{};

		XMVECTOR N{};
	};

	struct SClosestEdge
	{
		SClosestEdge() {};
		SClosestEdge(const XMVECTOR& _V0, const XMVECTOR& _V1) : V0{ _V0 }, V1{ _V1 } {};

		XMVECTOR V0{};
		XMVECTOR V1{};

		XMVECTOR M{};
	};

	struct SCollisionPair
	{
		SCollisionPair() {};
		SCollisionPair(ComponentIndexType _A, ComponentIndexType _B) : A{ _A }, B{ _B } {};

		ComponentIndexType A{};
		ComponentIndexType B{};
	};
	
	struct SCollisionData
	{
		SCollisionData() {};
		SCollisionData(JWEntity* _PtrEntityA, JWEntity* _PtrEntityB, const XMVECTOR& _CollisionNormal,
			float _PenetrationDepth, float _ClosingSpeed) :
			PtrEntityA{ _PtrEntityA }, PtrEntityB{ _PtrEntityB }, CollisionNormal{ _CollisionNormal },
			PenetrationDepth{ _PenetrationDepth }, ClosingSpeed{ _ClosingSpeed } {};

		JWEntity*	PtrEntityA{};
		JWEntity*	PtrEntityB{};
		
		XMVECTOR	CollisionNormal{};
		float		PenetrationDepth{};
		float		ClosingSpeed{};
	};

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
		// If mass is not set, it is assumed to be a NON-physical entity.
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

		const auto& GetClosestPointA0() const noexcept { if (m_ClosestPointsA.size() > 0) { return m_ClosestPointsA[0]; } return KVectorZero; };
		const auto& GetClosestPointA1() const noexcept { if (m_ClosestPointsA.size() > 1) { return m_ClosestPointsA[1]; } return KVectorZero; };
		const auto& GetClosestPointA2() const noexcept { if (m_ClosestPointsA.size() > 2) { return m_ClosestPointsA[2]; } return KVectorZero; };
		const auto& GetClosestPointB0() const noexcept { if (m_ClosestPointsB.size() > 0) { return m_ClosestPointsB[0]; } return KVectorZero; };
		const auto& GetClosestPointB1() const noexcept { if (m_ClosestPointsB.size() > 1) { return m_ClosestPointsB[1]; } return KVectorZero; };
		const auto& GetClosestPointB2() const noexcept { if (m_ClosestPointsB.size() > 2) { return m_ClosestPointsB[2]; } return KVectorZero; };
		const auto& GetClosestFaceA() const noexcept { return m_ClosestFaceA; };
		const auto& GetClosestFaceB() const noexcept { return m_ClosestFaceB; };
		auto IsThereAnyActualCollision() const noexcept { return m_IsThereAnyActualCollision; };
		const auto GetPenetrationDepth() const noexcept 
		{
			if (m_FineCollisionList.size() > 0) 
			{
				return m_FineCollisionList.front().PenetrationDepth;
			}
			return 0.0f;
		};

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

		auto IsPointAInB(const XMVECTOR& PointA, const VECTOR<SIndexTriangle>& BFaces, const XMMATRIX& BWorld,
			const VECTOR<size_t>& BVertexToPosition, const VECTOR<XMVECTOR>& BPositions) noexcept->bool;

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

		bool						m_IsThereAnyActualCollision{ false };
		VECTOR<SCollisionPair>		m_CoarseCollisionList{};
		VECTOR<SCollisionData>		m_FineCollisionList{};

		// Varaibles below are for debugging purpose
		VECTOR<XMVECTOR>			m_ClosestPointsA{};
		VECTOR<SClosestPoint>		m_ClosestPointsAIndex{};
		VECTOR<XMVECTOR>			m_ClosestPointsB{};
		VECTOR<SClosestPoint>		m_ClosestPointsBIndex{};
		SClosestFace				m_ClosestFaceA{};
		SClosestFace				m_ClosestFaceB{};
		

		JWFlagSystemPhysicsOption	m_FlagSystemPhyscisOption{ JWFlagSystemPhysicsOption_ApplyForces };
	};
};

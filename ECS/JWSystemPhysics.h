#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	// Gravity on Earth = 9.8m/s^2
	static constexpr float		KDefaultRestitution{ 0.8f };
	static constexpr float		KDefaultDamping{ 0.98f };
	static constexpr float		KNonPhysicalObjectInverseMass{ -1.0f };
	static constexpr float		KGravityOnEarth{ 9.8f };
	//static constexpr float		KGravityOnEarth{ 0.8f };
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
		SClosestPoint(float _Distance, const XMVECTOR& _Point) :
			Distance{ _Distance }, Point{ _Point } {};

		float		Distance{};
		XMVECTOR	Point{};
	};

	struct STransformedFace
	{
		STransformedFace() {};
		STransformedFace(const XMVECTOR& _V0, const XMVECTOR& _V1, const XMVECTOR& _V2, const XMVECTOR& _N, const XMVECTOR& _M) :
			V0{ _V0 }, V1{ _V1 }, V2{ _V2 }, N{ _N }, M{ _M } {};

		XMVECTOR	V0{};
		XMVECTOR	V1{};
		XMVECTOR	V2{};

		XMVECTOR	N{};
		XMVECTOR	M{};
	};
	
	struct SClosestFace
	{
		SClosestFace() {};
		SClosestFace(float _Dist, float _Dist2,
			const XMVECTOR& _V0, const XMVECTOR& _V1, const XMVECTOR& _V2, 
			const XMVECTOR& _N, const XMVECTOR& _Projected) :
			Dist{ _Dist }, Dist2{ _Dist2 }, V0{ _V0 }, V1{ _V1 }, V2{ _V2 }, N{ _N }, Projected{ _Projected } {};

		float		Dist{};
		float		Dist2{};

		XMVECTOR	V0{};
		XMVECTOR	V1{};
		XMVECTOR	V2{};

		XMVECTOR	N{};
		XMVECTOR	Projected{};
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
		SCollisionData(JWEntity* _PtrEntityA, JWEntity* _PtrEntityB, const XMVECTOR& _DirectionBA, const XMVECTOR& _CollisionNormal,
			float _PenetrationDepth, float _ClosingSpeed) :
			PtrEntityA{ _PtrEntityA }, PtrEntityB{ _PtrEntityB }, DirectionBA{ _DirectionBA }, CollisionNormal{ _CollisionNormal },
			PenetrationDepth{ _PenetrationDepth }, ClosingSpeed{ _ClosingSpeed } {};

		JWEntity*	PtrEntityA{};
		JWEntity*	PtrEntityB{};
		
		XMVECTOR	DirectionBA{};

		// @important:
		// Collision normal is always in b-a direction.
		XMVECTOR	CollisionNormal{};

		float		PenetrationDepth{};
		float		ClosingSpeed{};
	};

	enum class EPhysicsMaterial
	{
		UserDefined = 0,

		Wood,	// Fs middle,	Fk = Fs/1.88	{ 0.5, 0.26 }
		Iron,	// Fs v high,	Fk = Fs/6.66	{ 1.0, 0.15 }
		Steel,	// Fs high,		Fk = Fs/1.29	{ 0.3, 0.23 }
		Glass,	// Fs high,		Fk = Fs/2.35	{ 0.9, 0.38 }
		Ice,	// Fs v low,	Fk = Fs/3.33	{ 0.1, 0.03 }

		Ceramic,	// { 0.8, 0.31 } ??

		//Leather,	// Fs high,		Fk = Fs/??
		//Rubber,	// Fs v high,	Fk = Fs/1.4??
	};

	struct SMaterialFrictionData
	{
		SMaterialFrictionData() {};
		SMaterialFrictionData(EPhysicsMaterial _Material, const char* _UserDefinedMaterialName, float _StaticFrictionConstant, float _KineticFrictionConstant) :
			Material{ _Material }, StaticFrictionConstant{ _StaticFrictionConstant }, KineticFrictionConstant{ _KineticFrictionConstant }
		{
			if (_UserDefinedMaterialName != nullptr)
			{
				strcpy_s(UserDefinedMaterialName, _UserDefinedMaterialName);
			}
		};

		EPhysicsMaterial	Material{};
		char				UserDefinedMaterialName[16]{};

		float	StaticFrictionConstant{};
		float	KineticFrictionConstant{};

		bool operator==(const char* _UserDefinedMaterialName)
		{
			if (strcmp(UserDefinedMaterialName, _UserDefinedMaterialName) == 0)
			{
				return true;
			}
			return false;
		}

		bool operator==(EPhysicsMaterial _Material)
		{
			if (Material == _Material)
			{
				return true;
			}
			return false;
		}
	};

	static const SMaterialFrictionData KMaterialFrictionWood{ SMaterialFrictionData(EPhysicsMaterial::Wood, nullptr, 0.5f, 0.26f) };
	static const SMaterialFrictionData KMaterialFrictionIron{ SMaterialFrictionData(EPhysicsMaterial::Iron, nullptr, 1.0f, 0.15f) };
	static const SMaterialFrictionData KMaterialFrictionSteel{ SMaterialFrictionData(EPhysicsMaterial::Steel, nullptr, 0.3f, 0.23f) };
	static const SMaterialFrictionData KMaterialFrictionGlass{ SMaterialFrictionData(EPhysicsMaterial::Glass, nullptr, 0.9f, 0.38f) };
	static const SMaterialFrictionData KMaterialFrictionIce{ SMaterialFrictionData(EPhysicsMaterial::Ice, nullptr, 0.1f, 0.03f) };
	static const SMaterialFrictionData KMaterialFrictionCeramic{ SMaterialFrictionData(EPhysicsMaterial::Ceramic, nullptr, 0.8f, 0.31f) };

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

		// [Property]	coefficient of restitution
		// [Range]		[0.0f, 1.0f]
		float		Restitution{ KDefaultRestitution };

		// [Property]	material friction data
		// @default friction data is KMaterialFrictionWood
		SMaterialFrictionData MaterialFriction{ KMaterialFrictionWood };

		// [Property]	damping
		float		Damping{ KDefaultDamping };

		// @important: AccumulatedForce must be zeroed every frame.
		XMVECTOR	AccumulatedForce{};

		// @important: AccumulatedImpulse must be zeroed every frame.
		XMVECTOR	AccumulatedImpulse{};

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

		void SetMassByKilogram(float Kg) noexcept
		{
			SetMassByGram(Kg * 1000.0f);
		}
		
		void SetMassToInfinite() noexcept
		{
			InverseMass = 0;
		}

		void SetVelocity(const XMVECTOR& _Velocity) noexcept
		{
			Velocity = _Velocity;
		}

		void SetAcceleration(const XMVECTOR& _Acceleration) noexcept
		{
			Acceleration = _Acceleration;
		}

		void AddForce(const XMVECTOR& Force) noexcept
		{
			AccumulatedForce += Force;
		}

		void AddImpulse(const XMVECTOR& Impulse) noexcept
		{
			AccumulatedImpulse += Impulse;
		}

		void ClearAccumulation() noexcept
		{
			AccumulatedForce = KVectorZero;
			AccumulatedImpulse = KVectorZero;
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

		void AddMaterialFrictionData(const STRING& MaterialName, float StaticFrictionConstant, float KineticFrictionConstant);
		auto GetMaterialFrictionDataByMaterialName(const STRING& MaterialName)->const SMaterialFrictionData&;
		auto GetMaterialFrictionData(EPhysicsMaterial Material)->const SMaterialFrictionData&;

		auto PickEntity() noexcept->bool;

		void SetSystemPhysicsFlag(JWFlagSystemPhysicsOption Flag) noexcept;
		void ToggleSystemPhysicsFlag(JWFlagSystemPhysicsOption Flag) noexcept;

		const auto GetPickedEntity() const noexcept { return m_pPickedEntity; };
		auto GetPickedEntityName() const noexcept->const STRING&;

		const auto& GetPickingRayOrigin() const noexcept { return m_PickingRayOrigin; };
		const auto& GetPickingRayDirection() const noexcept { return m_PickingRayDirection; };

		const auto& GetPickedTriangleVertex(uint32_t PositionIndex) const noexcept { return m_PickedTriangle[min(PositionIndex, 2)]; };
		const auto& GetPickedPoint() const noexcept { return m_PickedPoint; };

		const auto& GetClosestPointA0() const noexcept { if (m_ClosestPointsA.size() > 0) { return m_ClosestPointsA[0].Point; } return KVectorZero; };
		const auto& GetClosestPointA1() const noexcept { if (m_ClosestPointsA.size() > 1) { return m_ClosestPointsA[1].Point; } return KVectorZero; };
		const auto& GetClosestPointA2() const noexcept { if (m_ClosestPointsA.size() > 2) { return m_ClosestPointsA[2].Point; } return KVectorZero; };
		const auto& GetClosestPointB0() const noexcept { if (m_ClosestPointsB.size() > 0) { return m_ClosestPointsB[0].Point; } return KVectorZero; };
		const auto& GetClosestPointB1() const noexcept { if (m_ClosestPointsB.size() > 1) { return m_ClosestPointsB[1].Point; } return KVectorZero; };
		const auto& GetClosestPointB2() const noexcept { if (m_ClosestPointsB.size() > 2) { return m_ClosestPointsB[2].Point; } return KVectorZero; };
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

		auto IsPointAInB(const XMVECTOR& PointA, const VECTOR<STransformedFace>& BTransformedFaces) noexcept->bool;

		void ProcessCollision() noexcept;

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

		// Closest points and faces for collision detection
		VECTOR<SClosestPoint>		m_ClosestPointsA{};
		VECTOR<SClosestPoint>		m_ClosestPointsB{};
		SClosestPoint				m_ClosestPointA{};
		SClosestPoint				m_ClosestPointB{};
		VECTOR<SClosestFace>		m_ClosestFacesA{};
		VECTOR<SClosestFace>		m_ClosestFacesB{};
		SClosestFace				m_ClosestFaceA{};
		SClosestFace				m_ClosestFaceB{};
		VECTOR<STransformedFace>	m_TransformedFacesA{};
		VECTOR<STransformedFace>	m_TransformedFacesB{};

		// Friction data
		VECTOR<SMaterialFrictionData>	m_vMaterialFrictionData{ KMaterialFrictionWood, KMaterialFrictionIron, KMaterialFrictionSteel,
			KMaterialFrictionGlass, KMaterialFrictionIce, KMaterialFrictionCeramic };

		JWFlagSystemPhysicsOption	m_FlagSystemPhyscisOption{ JWFlagSystemPhysicsOption_ApplyForces };
	};
};

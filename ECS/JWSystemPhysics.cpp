#include "JWECS.h"
#include "../Core/JWMath.h"

using namespace JWEngine;

void JWSystemPhysics::Create(JWECS& ECS, HWND hWnd, const SSize2& WindowSize) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;

	m_hWnd = hWnd;

	m_pWindowSize = &WindowSize;
}

void JWSystemPhysics::Destroy() noexcept {}

PRIVATE auto JWSystemPhysics::CreateComponent(EntityIndexType EntityIndex) noexcept->ComponentIndexType
{
	auto component_index{ static_cast<ComponentIndexType>(m_vComponents.size()) };

	// @important
	// Save component ID & pointer to Entity
	m_vComponents.emplace_back(EntityIndex, component_index);

	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(EntityIndex);

	// Set world matrix of bounding ellipsoid
	auto transform = ptr_entity->GetComponentTransform();
	const auto& bounding_sphere = m_vComponents[component_index].BoundingSphere;

	auto temp_center = bounding_sphere.Center;
	if (transform) { temp_center += transform->Position; }
	
	auto mat_scaling = XMMatrixScaling(bounding_sphere.Radius, bounding_sphere.Radius, bounding_sphere.Radius);
	auto mat_translation = XMMatrixTranslationFromVector(temp_center);

	/// Add bounding ellipsoid instance into SystemRender
	///m_pECS->SystemRender().AddBoundingEllipsoidInstance(mat_scaling * mat_translation);

	// Add bounding sphere instance into SystemRender
	m_pECS->SystemRender().AddBoundingSphereInstance(mat_scaling * mat_translation);

	return component_index;
}

PRIVATE void JWSystemPhysics::DestroyComponent(ComponentIndexType ComponentIndex) noexcept
{
	if (m_vComponents.size() == 0)
	{
		JW_ERROR_ABORT("There is no component to destroy.");
	}

	// Save the target component's index.
	auto component_index{ ComponentIndex };

	// Get the last index of the component vector.
	auto last_index = static_cast<ComponentIndexType>(m_vComponents.size() - 1);

	// Get pointer to the entity with last_index's component.
	auto ptr_entity_last_component = m_pECS->GetEntityByIndex(m_vComponents[last_index].EntityIndex);

	// See if the component index is invalid.
	if (component_index > last_index)
	{
		JW_ERROR_ABORT("Invalid component index.");
	}

	/// Erase bounding ellipsoid instance in JWSystemRender
	///m_pECS->SystemRender().EraseBoundingEllipsoidInstance(component_index);

	// Erase bounding sphere instance in JWSystemRender
	m_pECS->SystemRender().EraseBoundingSphereInstance(component_index);

	// Swap the last element of the vector and the deleted element if necessary
	if (component_index < last_index)
	{
		m_vComponents[component_index] = std::move(m_vComponents[last_index]);
		m_vComponents[component_index].ComponentIndex = component_index; // @important

		ptr_entity_last_component->SetComponentPhysicsIndex(component_index); // @important
	}

	// Shrink the size of the vector.
	m_vComponents.pop_back();
}

PRIVATE auto JWSystemPhysics::GetComponentPtr(ComponentIndexType ComponentIndex) noexcept->SComponentPhysics*
{
	if (ComponentIndex >= m_vComponents.size())
	{
		return nullptr;
	}

	return &m_vComponents[ComponentIndex];
}

auto JWSystemPhysics::PickEntity() noexcept->bool
{
	m_pPickedEntity = nullptr;
	m_pPickedTerrainEntity = nullptr;
	m_pPickedNonTerrainEntity = nullptr;
	
	CastPickingRay();

	if (m_vComponents.size() == 0)
	{
		return false;
	}

	// Reset distances for comparison
	m_PickedTerrainDistance = KVectorMax;
	m_PickedNonTerrainDistance = KVectorMax;

	//if (PickEntityByEllipsoid())
	if (PickEntityBySphere())
	{
		if (m_pPickedTerrainEntity)
		{
			//if (PickSubBoundingEllipsoid(m_pPickedTerrainEntity))
			if (PickSubBoundingSphere(m_pPickedTerrainEntity))
			{
				PickTerrainTriangle();
			}
		}

		// Final picking (between Terrain and Non-Terrain entities)
		if (XMVector3Less(m_PickedNonTerrainDistance, m_PickedTerrainDistance))
		{
			m_PickedDistance = m_PickedNonTerrainDistance;
			m_pPickedEntity = m_pPickedNonTerrainEntity;

			m_PickedTriangle[0] = KVectorZero;
			m_PickedTriangle[1] = KVectorZero;
			m_PickedTriangle[2] = KVectorZero;
		}
		else
		{
			m_PickedDistance = m_PickedTerrainDistance;
			m_pPickedEntity = m_pPickedTerrainEntity;
		}

		if (m_pPickedEntity)
		{
			m_PickedPoint = m_PickingRayOrigin + m_PickedDistance * m_PickingRayDirection;
		}

		return true;
	}

	return false;
}

PRIVATE __forceinline void JWSystemPhysics::CastPickingRay() noexcept
{
	// Get mouse cursor position in screen space (in client rect)
	GetCursorPos(&m_MouseClientPosition);
	ScreenToClient(m_hWnd, &m_MouseClientPosition);

	// Normalize mouse cursor position
	// x = [-1.0, 1.0]
	// y = [-1.0, 1.0]
	m_NormalizedMousePosition.x = (static_cast<float>(m_MouseClientPosition.x) / m_pWindowSize->floatX()) * 2.0f - 1.0f;
	m_NormalizedMousePosition.y = (static_cast<float>(m_MouseClientPosition.y) / m_pWindowSize->floatY()) * 2.0f - 1.0f;

	const auto& MatrixView = m_pECS->SystemCamera().CurrentViewMatrix();
	const auto& MatrixProjection = m_pECS->SystemCamera().CurrentProjectionMatrix();

	// Origin of the picking ray
	// (x, y, z) = (0, 0, 0)
	m_PickingRayViewSpacePosition = XMVectorSet(0, 0, 0.001f, 0);

	// Direction of the picking ray
	// x = [0, ScreenWidth]
	// y = [0, ScreenHeight]
	// z = 1.0
	m_PickingRayViewSpaceDirection = XMVectorSet(
		+m_NormalizedMousePosition.x / XMVectorGetX(MatrixProjection.r[0]),
		-m_NormalizedMousePosition.y / XMVectorGetY(MatrixProjection.r[1]),
		1.0f,
		0.0f
	);

	auto MatrixViewInverse = XMMatrixInverse(nullptr, MatrixView);
	m_PickingRayOrigin = XMVector3TransformCoord(m_PickingRayViewSpacePosition, MatrixViewInverse);
	m_PickingRayDirection = XMVector3TransformNormal(m_PickingRayViewSpaceDirection, MatrixViewInverse);
}

/*
PRIVATE auto JWSystemPhysics::PickEntityByEllipsoid() noexcept->bool
{
	for (auto iter : m_vComponents)
	{
		auto ptr_entity = m_pECS->GetEntityByIndex(iter.EntityIndex);

		auto transform{ ptr_entity->GetComponentTransform() };
		auto type{ ptr_entity->GetEntityType() };

		if (type != EEntityType::UserDefined)
		{
			// MainSprite, MainTerrain need to be picked
			if (!(
				(type == EEntityType::MainSprite) ||
				(type == EEntityType::MainTerrain)
				))
			{
				continue;
			}
		}

		if (transform)
		{
			auto camera{ ptr_entity->GetComponentCamera() };
			if (camera)
			{
				// @important
				// You must be UNABLE to pick the current camera!
				if (m_pECS->SystemCamera().GetCurrentCameraComponentID() == camera->ComponentIndex)
				{
					continue;
				}
			}

			// Transform ray into ellipsoid space
			auto inv_ellipsoid_world = XMMatrixInverse(nullptr, iter.BoundingEllipsoid.EllipsoidWorld);
			auto e_ray_origin = XMVector3TransformCoord(m_PickingRayOrigin, inv_ellipsoid_world);
			auto e_ray_direction = XMVector3TransformNormal(m_PickingRayDirection, inv_ellipsoid_world);
			

			if (type == EEntityType::MainTerrain)
			{
				if (IntersectRayUnitSphere(e_ray_origin, e_ray_direction))
				{
					m_pPickedTerrainEntity = ptr_entity;
				}
			}
			else
			{
				if (IntersectRayUnitSphere(e_ray_origin, e_ray_direction, &m_PickedNonTerrainDistance))
				{
					m_pPickedNonTerrainEntity = ptr_entity;
				}
			}
		}
	}

	if ((m_pPickedTerrainEntity) || (m_pPickedNonTerrainEntity))
	{
		return true;
	}

	return false;
}
*/

PRIVATE auto JWSystemPhysics::PickEntityBySphere() noexcept->bool
{
	for (auto iter : m_vComponents)
	{
		auto ptr_entity = m_pECS->GetEntityByIndex(iter.EntityIndex);

		auto transform{ ptr_entity->GetComponentTransform() };
		auto type{ ptr_entity->GetEntityType() };

		if (type != EEntityType::UserDefined)
		{
			// MainSprite, MainTerrain need to be picked
			if (!(
				(type == EEntityType::MainSprite) ||
				(type == EEntityType::MainTerrain)
				))
			{
				continue;
			}
		}

		if (transform)
		{
			auto camera{ ptr_entity->GetComponentCamera() };
			if (camera)
			{
				// @important
				// You must be UNABLE to pick the current camera!
				if (m_pECS->SystemCamera().GetCurrentCameraComponentID() == camera->ComponentIndex)
				{
					continue;
				}
			}

			auto physics = ptr_entity->GetComponentPhysics();
			if (physics)
			{
				auto world_center = physics->BoundingSphere.Center;
				if (transform)
				{
					world_center += transform->Position;
				}
				if (type == EEntityType::MainTerrain)
				{
					if (IntersectRaySphere(m_PickingRayOrigin, m_PickingRayDirection, physics->BoundingSphere.Radius, world_center))
					{
						m_pPickedTerrainEntity = ptr_entity;
					}
				}
				else
				{
					if (IntersectRaySphere(m_PickingRayOrigin, m_PickingRayDirection, physics->BoundingSphere.Radius, world_center,
						&m_PickedNonTerrainDistance))
					{
						m_pPickedNonTerrainEntity = ptr_entity;
					}
				}
			}
		}
	}

	if ((m_pPickedTerrainEntity) || (m_pPickedNonTerrainEntity))
	{
		return true;
	}

	return false;
}

/*
PRIVATE auto JWSystemPhysics::PickSubBoundingEllipsoid(JWEntity* PtrEntity) noexcept->bool
{
	auto physics = PtrEntity->GetComponentPhysics();
	if (physics == nullptr) { return false; }
	auto transform = PtrEntity->GetComponentTransform();
	if (transform == nullptr) { return false; }

	// Clear array
	m_vPickedSubBoundingEllipsoidID.clear();

	auto& sub_bounding_ellipsoids = physics->SubBoundingEllipsoids;

	static XMMATRIX inv_ellipsoid_world{};
	static XMVECTOR e_ray_origin{};
	static XMVECTOR e_ray_direction{};

	for (uint32_t id = static_cast<uint32_t>(sub_bounding_ellipsoids.size()); id > 0; --id)
	{
		// Transform ray into ellipsoid space
		inv_ellipsoid_world = XMMatrixInverse(nullptr, sub_bounding_ellipsoids[id - 1].EllipsoidWorld);
		e_ray_origin = XMVector3TransformCoord(m_PickingRayOrigin, inv_ellipsoid_world);
		e_ray_direction = XMVector3TransformNormal(m_PickingRayDirection, inv_ellipsoid_world);

		// @important: NO distance comparison!
		if (IntersectRayUnitSphere(e_ray_origin, e_ray_direction))
		{
			m_vPickedSubBoundingEllipsoidID.push_back(id - 1);
		}
	}

	if (m_vPickedSubBoundingEllipsoidID.size())
	{
		return true;
	}

	return false;
}
*/

PRIVATE auto JWSystemPhysics::PickSubBoundingSphere(JWEntity* PtrEntity) noexcept->bool
{
	auto physics = PtrEntity->GetComponentPhysics();
	if (physics == nullptr) { return false; }
	auto transform = PtrEntity->GetComponentTransform();
	if (transform == nullptr) { return false; }

	// Clear array
	m_vPickedSubBoundingSphereID.clear();

	auto& sub_bounding_spheres = physics->SubBoundingSpheres;

	for (auto id = static_cast<uint32_t>(sub_bounding_spheres.size()); id > 0; --id)
	{
		auto world_center = sub_bounding_spheres[id - 1].Center + transform->Position;

		// @important: NO distance comparison!
		if (IntersectRaySphere(m_PickingRayOrigin, m_PickingRayDirection, sub_bounding_spheres[id - 1].Radius, world_center))
		{
			m_vPickedSubBoundingSphereID.push_back(id - 1);
		}
	}

	if (m_vPickedSubBoundingSphereID.size())
	{
		return true;
	}

	return false;
}

PRIVATE void JWSystemPhysics::PickTerrainTriangle() noexcept
{
	if (m_pPickedTerrainEntity)
	{
		auto render = m_pPickedTerrainEntity->GetComponentRender();
		if (render == nullptr) { return; }

		auto ptr_terrain = render->PtrTerrain;
		if (ptr_terrain == nullptr) { return; }

		auto transform = m_pPickedTerrainEntity->GetComponentTransform();
		
		XMVECTOR v[3]{};
		auto picked_distance{ KVectorMax };

		for (auto& bs : m_vPickedSubBoundingSphereID)
		{
			for (auto& node : ptr_terrain->QuadTree)
			{
				if (node.SubBoundingVolumeID == bs)
				{
					const auto& faces{ node.IndexData.vFaces };
					const auto& vertices{ node.VertexData.vVerticesModel };
					for (auto triangle : faces)
					{
						if (transform)
						{
							// Move vertices from local space to world space!
							v[0] = XMVector3TransformCoord(vertices[triangle._0].Position, transform->WorldMatrix);
							v[1] = XMVector3TransformCoord(vertices[triangle._1].Position, transform->WorldMatrix);
							v[2] = XMVector3TransformCoord(vertices[triangle._2].Position, transform->WorldMatrix);
						}

						if (IntersectRayTriangle(m_PickedPoint, picked_distance,
							m_PickingRayOrigin, m_PickingRayDirection,
							v[0], v[1], v[2]))
						{
							m_PickedTriangle[0] = v[0];
							m_PickedTriangle[1] = v[1];
							m_PickedTriangle[2] = v[2];
						}
					}
				}
			}
		}

		m_PickedTerrainDistance = picked_distance;
	}
}

auto JWSystemPhysics::GetPickedEntityName() const noexcept->const STRING&
{
	if (m_pPickedEntity) 
	{
		return m_pPickedEntity->GetEntityName();
	}

	return KNoName;
};

void JWSystemPhysics::SetSystemPhysicsFlag(JWFlagSystemPhysicsOption Flag) noexcept
{
	m_FlagSystemPhyscisOption = Flag;
}

void JWSystemPhysics::ToggleSystemPhysicsFlag(JWFlagSystemPhysicsOption Flag) noexcept
{
	m_FlagSystemPhyscisOption ^= Flag;
}

void JWSystemPhysics::ApplyUniversalGravity() noexcept
{
	if (m_FlagSystemPhyscisOption & JWFlagSystemPhysicsOption_ApplyForces)
	{
		ApplyUniversalAcceleration(KVectorGravityOnEarth);
	}
}

void JWSystemPhysics::ApplyUniversalAcceleration(const XMVECTOR& _Acceleration) noexcept
{
	for (auto& iter : m_vComponents)
	{
		if (iter.InverseMass > 0)
		{
			iter.AccumulatedForce += _Acceleration / iter.InverseMass;
		}
	}
}

void JWSystemPhysics::ZeroAllVelocities() noexcept
{
	for (auto& iter : m_vComponents)
	{
		if (iter.InverseMass > 0)
		{
			iter.Velocity = KVectorZero;
		}
	}
}

void JWSystemPhysics::Execute() noexcept
{
	for (auto& iter : m_vComponents)
	{
		// Get pointer to the entity.
		auto ptr_entity = m_pECS->GetEntityByIndex(iter.EntityIndex);

		auto transform{ ptr_entity->GetComponentTransform() };
		if (transform)
		{
			if (iter.InverseMass > 0)
			{
				auto delta_time = m_pECS->GetDeltaTime();
				assert(delta_time > 0);

				// a = 1/m * f;
				iter.Acceleration = iter.InverseMass * iter.AccumulatedForce;

				// v' = v + at
				iter.Velocity += iter.Acceleration * delta_time;

				if (XMVectorGetY(transform->Position) < KPhysicsWorldFloor)
				{
					iter.Velocity = KVectorZero;
				}

				// p' = p + vt
				transform->Position += iter.Velocity * delta_time;

				// Reset accumulated force
				iter.AccumulatedForce = KVectorZero;
			}

			/// Update bounding ellipsoid
			///UpdateBoundingEllipsoid(iter);

			/// Update sub-bounding ellipsoids
			///UpdateSubBoundingEllipsoids(iter);

			// Update bounding sphere
			UpdateBoundingSphere(iter);

			// Update sub-bounding spheres
			UpdateSubBoundingSpheres(iter);
		}
	}

	// Collision detection
	DetectCoarseCollision();

	DetectFineCollision();
}

/*
PRIVATE void JWSystemPhysics::UpdateBoundingEllipsoid(SComponentPhysics& Physics) noexcept
{
	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(Physics.EntityIndex);

	// Calculate world matrix of the bounding ellipsoid
	auto transform = ptr_entity->GetComponentTransform();
	auto& bounding_ellipsoid = Physics.BoundingEllipsoid;

	auto temp_center = bounding_ellipsoid.Offset;
	if (transform) { temp_center += transform->Position; }

	auto mat_scaling = XMMatrixScaling(bounding_ellipsoid.RadiusX, bounding_ellipsoid.RadiusY, bounding_ellipsoid.RadiusZ);
	auto mat_translation = XMMatrixTranslationFromVector(temp_center);
	bounding_ellipsoid.EllipsoidWorld = mat_scaling * mat_translation;

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingEllipsoidInstance(Physics.ComponentIndex, bounding_ellipsoid.EllipsoidWorld);
}

PRIVATE void JWSystemPhysics::UpdateSubBoundingEllipsoids(SComponentPhysics& Physics) noexcept
{
	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(Physics.EntityIndex);

	// Calculate world matrix of the sub-bounding ellipsoids
	auto transform = ptr_entity->GetComponentTransform();
	auto entity_position = transform->Position;
	for (auto& curr_sub_be : Physics.SubBoundingEllipsoids)
	{
		auto mat_scaling = XMMatrixScaling(curr_sub_be.RadiusX, curr_sub_be.RadiusY, curr_sub_be.RadiusZ);
		auto mat_translation = XMMatrixTranslationFromVector(entity_position + curr_sub_be.Offset);
		curr_sub_be.EllipsoidWorld = mat_scaling * mat_translation;
	}
}
*/

PRIVATE void JWSystemPhysics::UpdateBoundingSphere(SComponentPhysics& Physics) noexcept
{
	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(Physics.EntityIndex);

	// Calculate world matrix of the bounding ellipsoid
	auto transform = ptr_entity->GetComponentTransform();
	auto& bounding_sphere = Physics.BoundingSphere;

	auto temp_center = bounding_sphere.Center;
	if (transform) { temp_center += transform->Position; }

	auto mat_scaling = XMMatrixScaling(bounding_sphere.Radius, bounding_sphere.Radius, bounding_sphere.Radius);
	auto mat_translation = XMMatrixTranslationFromVector(temp_center);

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingSphereInstance(Physics.ComponentIndex, mat_scaling * mat_translation);
}

PRIVATE void JWSystemPhysics::UpdateSubBoundingSpheres(SComponentPhysics& Physics) noexcept
{
	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(Physics.EntityIndex);

	// Calculate world matrix of the sub-bounding ellipsoids
	auto transform = ptr_entity->GetComponentTransform();
	auto entity_position = transform->Position;
	for (auto& curr_sub_bs : Physics.SubBoundingSpheres)
	{
		auto mat_scaling = XMMatrixScaling(curr_sub_bs.Radius, curr_sub_bs.Radius, curr_sub_bs.Radius);
		auto mat_translation = XMMatrixTranslationFromVector(entity_position + curr_sub_bs.Center);
	}
}

PRIVATE void JWSystemPhysics::DetectCoarseCollision() noexcept
{
	auto n = m_vComponents.size();
	if (n == 0) { return; }

	auto max_capacity = n * (n - 1) / 2;
	if (m_CoarseCollisionList.capacity() < max_capacity)
	{
		m_CoarseCollisionList.reserve(max_capacity);
	}
	m_CoarseCollisionList.clear();

	for (auto i = m_vComponents.cbegin(); i != m_vComponents.cend(); ++i)
	{
		if (i->InverseMass != KNonPhysicalObjectInverseMass)
		{
			auto i_entity = m_pECS->GetEntityByIndex(i->EntityIndex);
			auto i_transform = i_entity->GetComponentTransform();
			auto i_world_center = i->BoundingSphere.Center;
			if (i_transform)
			{
				i_world_center += i_transform->Position;
			}

			for (auto j = i + 1; j != m_vComponents.cend(); ++j)
			{
				if (j->InverseMass != KNonPhysicalObjectInverseMass)
				{
					auto j_entity = m_pECS->GetEntityByIndex(j->EntityIndex);
					auto j_transform = j_entity->GetComponentTransform();
					auto j_world_center = j->BoundingSphere.Center;
					if (j_transform)
					{
						j_world_center += j_transform->Position;
					}

					if (IntersectSpheres(i->BoundingSphere.Radius, i_world_center, j->BoundingSphere.Radius, j_world_center))
					{
						m_CoarseCollisionList.emplace_back(i->ComponentIndex, j->ComponentIndex);
					}
				}
			}
		}
	}
}

bool ClosestPointPred(const SClosestPoint& a, const SClosestPoint& b)
{
	return a.Distance > b.Distance;
}

PRIVATE void JWSystemPhysics::DetectFineCollision() noexcept
{
	if (m_CoarseCollisionList.size() == 0) { return; }

	m_IsThereAnyActualCollision = false;

	for (const auto& iter : m_CoarseCollisionList)
	{
		const auto& a_physics = m_vComponents[iter.A];
		const auto& b_physics = m_vComponents[iter.B];

		auto& a_collision_mesh{ a_physics.PtrCollisionMesh };
		auto& b_collision_mesh{ b_physics.PtrCollisionMesh };
		if ((a_collision_mesh == nullptr) || (b_collision_mesh == nullptr))
		{
			JW_ERROR_ABORT("The collision mesh is missing.");
		}
		
		const auto& a_positions = a_collision_mesh->vPositionVertex;
		const auto& b_positions = b_collision_mesh->vPositionVertex;
		const auto& a_faces = a_collision_mesh->ModelData.IndexData.vFaces;
		const auto& b_faces = b_collision_mesh->ModelData.IndexData.vFaces;
		const auto& a_v_to_pv = a_collision_mesh->vPositionVertexIndexFromVertexIndex;
		const auto& b_v_to_pv = b_collision_mesh->vPositionVertexIndexFromVertexIndex;
		const auto& a_face_with_position = a_collision_mesh->vFaceWithPositionVertex;
		const auto& b_face_with_position = b_collision_mesh->vFaceWithPositionVertex;
		
		// #1 Get world (mass) center of objects
		const auto& a_transform = m_pECS->GetEntityByIndex(a_physics.EntityIndex)->GetComponentTransform();
		const auto& b_transform = m_pECS->GetEntityByIndex(b_physics.EntityIndex)->GetComponentTransform();
		auto a_center_world = a_physics.BoundingSphere.Center + a_transform->Position;
		auto b_center_world = b_physics.BoundingSphere.Center + b_transform->Position;

		// #2 Get relative directions
		auto ba_dir = XMVector3Normalize(a_center_world - b_center_world);
		auto ab_dir = -ba_dir;

		// #3 Get the closest points of each object between them
		// #3-1 Get closest points of object a
		m_ClosestPointsAIndex.clear();
		m_ClosestPointsAIndex.reserve(a_positions.size());
		for (size_t i = 0; i < a_positions.size(); ++i)
		{
			auto distance = XMVectorGetX(XMVector3Dot(a_positions[i], ab_dir));
			m_ClosestPointsAIndex.emplace_back(i, distance);
		}
		std::sort(m_ClosestPointsAIndex.begin(), m_ClosestPointsAIndex.end(), ClosestPointPred);

		m_ClosestPointsA.clear();
		m_ClosestPointsA.reserve(m_ClosestPointsAIndex.size());
		for (auto position_i : m_ClosestPointsAIndex)
		{
			m_ClosestPointsA.emplace_back(XMVector3TransformCoord(a_positions[position_i.PositionIndex], a_transform->WorldMatrix));
		}

		// #3-2 Get closest points of object b
		m_ClosestPointsBIndex.clear();
		m_ClosestPointsBIndex.reserve(b_positions.size());
		for (size_t i = 0; i < b_positions.size(); ++i)
		{
			auto distance = XMVectorGetX(XMVector3Dot(b_positions[i], ba_dir));
			m_ClosestPointsBIndex.emplace_back(i, distance);
		}
		std::sort(m_ClosestPointsBIndex.begin(), m_ClosestPointsBIndex.end(), ClosestPointPred);

		m_ClosestPointsB.clear();
		m_ClosestPointsB.reserve(m_ClosestPointsBIndex.size());
		for (auto position_i : m_ClosestPointsBIndex)
		{
			m_ClosestPointsB.emplace_back(XMVector3TransformCoord(b_positions[position_i.PositionIndex], b_transform->WorldMatrix));
		}

		// #4 See if the two objects intersect
		ECollisionType collision_type{ ECollisionType::None };
		bool are_in_contact{ false };
		bool is_point_a_in_face_b{ false };
		bool is_point_b_in_face_a{ false };

		// #4-1 Point - Face
		// #4-1-1 Point of a - Face of b
		{
			const auto& p_a = m_ClosestPointsA.front();
			float dist_ab{};
			ProjectPointOntoPlane(dist_ab, p_a, m_ClosestPointsB.front(), ba_dir);
			if (dist_ab < 0)
			{
				is_point_a_in_face_b = IsPointAInB(p_a, b_faces, b_transform->WorldMatrix, b_v_to_pv, b_positions);
			}
		}

		// #4-1-2 Point of b - Face of a
		if (is_point_a_in_face_b == false)
		{
			const auto& p_b = m_ClosestPointsB.front();
			float dist_ba{};
			ProjectPointOntoPlane(dist_ba, p_b, m_ClosestPointsA.front(), ab_dir);
			if (dist_ba < 0)
			{
				is_point_b_in_face_a = IsPointAInB(p_b, a_faces, a_transform->WorldMatrix, a_v_to_pv, a_positions);
			}
		}
		
		if ((is_point_b_in_face_a == true) || (is_point_a_in_face_b == true))
		{
			are_in_contact = true;
			collision_type = ECollisionType::PointFace;
		}

		// #4-2 Find the closest face of each object to another
		if (are_in_contact == false)
		{
			// Find the closest face of a
			XMVECTOR angle_max{ -KVectorMax };
			for (const auto& a_face : a_face_with_position[m_ClosestPointsAIndex.front().PositionIndex])
			{
				auto a0 = XMVector3TransformCoord(a_positions[a_v_to_pv[a_faces[a_face]._0]], a_transform->WorldMatrix);
				auto a1 = XMVector3TransformCoord(a_positions[a_v_to_pv[a_faces[a_face]._1]], a_transform->WorldMatrix);
				auto a2 = XMVector3TransformCoord(a_positions[a_v_to_pv[a_faces[a_face]._2]], a_transform->WorldMatrix);
				auto n = GetTriangleNormal(a0, a1, a2);

				auto angle = XMVector3Dot(n, ab_dir);
				if (XMVector3Greater(angle, angle_max))
				{
					angle_max = angle;
					m_ClosestFaceA = SClosestFace(a0, a1, a2);
				}
			}

			// Find the closest face of b
			angle_max = -KVectorMax;
			for (const auto& b_face : b_face_with_position[m_ClosestPointsBIndex.front().PositionIndex])
			{
				auto b0 = XMVector3TransformCoord(b_positions[b_v_to_pv[b_faces[b_face]._0]], b_transform->WorldMatrix);
				auto b1 = XMVector3TransformCoord(b_positions[b_v_to_pv[b_faces[b_face]._1]], b_transform->WorldMatrix);
				auto b2 = XMVector3TransformCoord(b_positions[b_v_to_pv[b_faces[b_face]._2]], b_transform->WorldMatrix);
				auto n = GetTriangleNormal(b0, b1, b2);

				auto angle = XMVector3Dot(n, ba_dir);
				if (XMVector3Greater(angle, angle_max))
				{
					angle_max = angle;
					m_ClosestFaceB = SClosestFace(b0, b1, b2);
				}
			}
		}

		// #4-3 Edge - Edge
		bool is_edge_b_through_face_a{ false };
		bool is_edge_a_through_face_b{ false };
		EClosestEdgePair edge_a{ EClosestEdgePair::None };
		EClosestEdgePair edge_b{ EClosestEdgePair::None };

		// #4-3-1 Edge of 'b' is intersecting the closest face of 'a'
		// Find the edge of 'a' that intersects 'b'
		if (are_in_contact == false)
		{
			XMVECTOR closest_point_of_edge_a_to_b{};

			// Edge a V0-V1
			if (ProjectPointOntoSegment(b_center_world, m_ClosestFaceA.V0, m_ClosestFaceA.V1, closest_point_of_edge_a_to_b))
			{
				if (is_edge_b_through_face_a = IsPointAInB(closest_point_of_edge_a_to_b, b_faces, b_transform->WorldMatrix, b_v_to_pv, b_positions))
				{
					edge_a = EClosestEdgePair::V0V1;
				}
			}

			// Edge a V0-V2
			if (edge_a == EClosestEdgePair::None)
			{
				if (ProjectPointOntoSegment(b_center_world, m_ClosestFaceA.V0, m_ClosestFaceA.V2, closest_point_of_edge_a_to_b))
				{
					if (is_edge_b_through_face_a = IsPointAInB(closest_point_of_edge_a_to_b, b_faces, b_transform->WorldMatrix, b_v_to_pv, b_positions))
					{
						edge_a = EClosestEdgePair::V0V2;
					}
				}
			}

			// Edge a V1-V2
			if (edge_a == EClosestEdgePair::None)
			{
				if (ProjectPointOntoSegment(b_center_world, m_ClosestFaceA.V1, m_ClosestFaceA.V2, closest_point_of_edge_a_to_b))
				{
					if (is_edge_b_through_face_a = IsPointAInB(closest_point_of_edge_a_to_b, b_faces, b_transform->WorldMatrix, b_v_to_pv, b_positions))
					{
						edge_a = EClosestEdgePair::V1V2;
					}
				}
			}

			// Find the edge of 'b' that intersects the closest face of 'a'
			if (edge_a != EClosestEdgePair::None)
			{
				// Edge b V0-V1
				XMVECTOR intersected_point{};
				if (
					(IntersectRayTriangle(
						intersected_point, m_ClosestFaceB.V0, GetRayDirection(m_ClosestFaceB.V0, m_ClosestFaceB.V1),
						m_ClosestFaceA.V0, m_ClosestFaceA.V1, m_ClosestFaceA.V2) == true)
					||
					(IntersectRayTriangle(
						intersected_point, m_ClosestFaceB.V1, GetRayDirection(m_ClosestFaceB.V1, m_ClosestFaceB.V0),
						m_ClosestFaceA.V0, m_ClosestFaceA.V1, m_ClosestFaceA.V2) == true)
					)
				{
					edge_b = EClosestEdgePair::V0V1;
				}

				// Edge b V0-V2
				if (edge_b == EClosestEdgePair::None)
				{
					if (
						(IntersectRayTriangle(
							intersected_point, m_ClosestFaceB.V0, GetRayDirection(m_ClosestFaceB.V0, m_ClosestFaceB.V2),
							m_ClosestFaceA.V0, m_ClosestFaceA.V1, m_ClosestFaceA.V2) == true)
						||
						(IntersectRayTriangle(
							intersected_point, m_ClosestFaceB.V2, GetRayDirection(m_ClosestFaceB.V2, m_ClosestFaceB.V0),
							m_ClosestFaceA.V0, m_ClosestFaceA.V1, m_ClosestFaceA.V2) == true)
						)
					{
						edge_b = EClosestEdgePair::V0V2;
					}
				}

				// Edge b V1-V2
				if (edge_b == EClosestEdgePair::None)
				{
					if (
						(IntersectRayTriangle(
							intersected_point, m_ClosestFaceB.V1, GetRayDirection(m_ClosestFaceB.V1, m_ClosestFaceB.V2),
							m_ClosestFaceA.V0, m_ClosestFaceA.V1, m_ClosestFaceA.V2) == true)
						||
						(IntersectRayTriangle(
							intersected_point, m_ClosestFaceB.V2, GetRayDirection(m_ClosestFaceB.V2, m_ClosestFaceB.V1),
							m_ClosestFaceA.V0, m_ClosestFaceA.V1, m_ClosestFaceA.V2) == true)
						)
					{
						edge_b = EClosestEdgePair::V1V2;
					}
				}
			}

			if (edge_a != EClosestEdgePair::None)
			{
				if (edge_b == EClosestEdgePair::None)
				{
					// To prevent robustness error,
					// arbitrarily assign an edge.
					edge_b = EClosestEdgePair::V0V1;
				}

				are_in_contact = true;
				collision_type = ECollisionType::EdgeEdge;
			}
		}

		// #4-3-2 Edge of 'a' is intersecting the closest face of 'b'
		// Find the edge of 'b' that intersects 'a'
		if (are_in_contact == false)
		{
			XMVECTOR closest_point_of_edge_b_to_a{};

			// Edge b V0-V1
			if (ProjectPointOntoSegment(a_center_world, m_ClosestFaceB.V0, m_ClosestFaceB.V1, closest_point_of_edge_b_to_a))
			{
				if (is_edge_a_through_face_b = IsPointAInB(closest_point_of_edge_b_to_a, a_faces, a_transform->WorldMatrix, a_v_to_pv, a_positions))
				{
					edge_b = EClosestEdgePair::V0V1;
				}
			}

			// Edge b V0-V2
			if (edge_b == EClosestEdgePair::None)
			{
				if (ProjectPointOntoSegment(a_center_world, m_ClosestFaceB.V0, m_ClosestFaceB.V2, closest_point_of_edge_b_to_a))
				{
					if (is_edge_a_through_face_b = IsPointAInB(closest_point_of_edge_b_to_a, a_faces, a_transform->WorldMatrix, a_v_to_pv, a_positions))
					{
						edge_b = EClosestEdgePair::V0V2;
					}
				}
			}

			// Edge b V1-V2
			if (edge_b == EClosestEdgePair::None)
			{
				if (ProjectPointOntoSegment(a_center_world, m_ClosestFaceB.V1, m_ClosestFaceB.V2, closest_point_of_edge_b_to_a))
				{
					if (is_edge_a_through_face_b = IsPointAInB(closest_point_of_edge_b_to_a, a_faces, a_transform->WorldMatrix, a_v_to_pv, a_positions))
					{
						edge_b = EClosestEdgePair::V1V2;
					}
				}
			}

			// Find the edge of 'a' that intersects the closest face of 'b'
			if (edge_b != EClosestEdgePair::None)
			{
				// Edge a V0-V1
				XMVECTOR intersected_point{};
				if (
					(IntersectRayTriangle(
						intersected_point, m_ClosestFaceA.V0, GetRayDirection(m_ClosestFaceA.V0, m_ClosestFaceA.V1),
						m_ClosestFaceB.V0, m_ClosestFaceB.V1, m_ClosestFaceB.V2) == true)
					||
					(IntersectRayTriangle(
						intersected_point, m_ClosestFaceA.V1, GetRayDirection(m_ClosestFaceA.V1, m_ClosestFaceA.V0),
						m_ClosestFaceB.V0, m_ClosestFaceB.V1, m_ClosestFaceB.V2) == true)
					)
				{
					edge_a = EClosestEdgePair::V0V1;
				}

				// Edge a V0-V2
				if (edge_a == EClosestEdgePair::None)
				{
					if (
						(IntersectRayTriangle(
							intersected_point, m_ClosestFaceA.V0, GetRayDirection(m_ClosestFaceA.V0, m_ClosestFaceA.V2),
							m_ClosestFaceB.V0, m_ClosestFaceB.V1, m_ClosestFaceB.V2) == true)
						||
						(IntersectRayTriangle(
							intersected_point, m_ClosestFaceA.V2, GetRayDirection(m_ClosestFaceA.V2, m_ClosestFaceA.V0),
							m_ClosestFaceB.V0, m_ClosestFaceB.V1, m_ClosestFaceB.V2) == true)
						)
					{
						edge_a = EClosestEdgePair::V0V2;
					}
				}

				// Edge a V1-V2
				if (edge_a == EClosestEdgePair::None)
				{
					if (
						(IntersectRayTriangle(
							intersected_point, m_ClosestFaceA.V1, GetRayDirection(m_ClosestFaceA.V1, m_ClosestFaceA.V2),
							m_ClosestFaceB.V0, m_ClosestFaceB.V1, m_ClosestFaceB.V2) == true)
						||
						(IntersectRayTriangle(
							intersected_point, m_ClosestFaceA.V2, GetRayDirection(m_ClosestFaceA.V2, m_ClosestFaceA.V1),
							m_ClosestFaceB.V0, m_ClosestFaceB.V1, m_ClosestFaceB.V2) == true)
						)
					{
						edge_a = EClosestEdgePair::V1V2;
					}
				}
			}

			if (edge_b != EClosestEdgePair::None)
			{
				if (edge_a == EClosestEdgePair::None)
				{
					// To prevent robustness error,
					// arbitrarily assign an edge.
					edge_a = EClosestEdgePair::V0V1;
				}

				are_in_contact = true;
				collision_type = ECollisionType::EdgeEdge;
			}
		}

		if (are_in_contact == true)
		{
			m_IsThereAnyActualCollision = true;

			// #5 Get signed closing speed
			auto relative_velocity_ba = b_physics.Velocity - a_physics.Velocity;
			auto signed_closing_speed = XMVector3Dot(relative_velocity_ba, ba_dir);
			if (XMVector3Greater(signed_closing_speed, KVectorZero))
			{
				// They are colliding, not separating.
				// #6 Get penetration....

			}
		}
		
	}
}

PRIVATE auto JWSystemPhysics::IsPointAInB(const XMVECTOR& PointA, const VECTOR<SIndexTriangle>& BFaces, const XMMATRIX& BWorld,
	const VECTOR<size_t>& BVertexToPosition, const VECTOR<XMVECTOR>& BPositions) noexcept->bool
{
	// return true if PointA is inside all faces of B
	bool result{ false };
	float dist{};

	for (auto b_face : BFaces)
	{
		auto b0 = XMVector3TransformCoord(BPositions[BVertexToPosition[b_face._0]], BWorld);
		auto b1 = XMVector3TransformCoord(BPositions[BVertexToPosition[b_face._1]], BWorld);
		auto b2 = XMVector3TransformCoord(BPositions[BVertexToPosition[b_face._2]], BWorld);
		auto n = GetTriangleNormal(b0, b1, b2);

		ProjectPointOntoPlane(dist, PointA, b0, n);

		if (dist < 0)
		{
			result = true;
		}
		else
		{
			return false;
		}
	}

	return result;
}
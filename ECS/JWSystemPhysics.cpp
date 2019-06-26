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

void JWSystemPhysics::AddMaterialFrictionData(const STRING& MaterialName, float StaticFrictionConstant, float KineticFrictionConstant)
{
	assert(MaterialName.size() < 16);

	m_vMaterialFrictionData.emplace_back(EPhysicsMaterial::UserDefined, MaterialName.c_str(), StaticFrictionConstant, KineticFrictionConstant);
}

auto JWSystemPhysics::GetMaterialFrictionDataByMaterialName(const STRING& MaterialName)->const SMaterialFrictionData&
{
	auto found = std::find(m_vMaterialFrictionData.begin(), m_vMaterialFrictionData.end(), MaterialName.c_str());
	if (found == m_vMaterialFrictionData.end())
	{
		JW_ERROR_ABORT("Cannot find mathcing material name.");
	}
	else
	{
		return *found;
	}
}

auto JWSystemPhysics::GetMaterialFrictionData(EPhysicsMaterial Material)->const SMaterialFrictionData&
{
	auto found = std::find(m_vMaterialFrictionData.begin(), m_vMaterialFrictionData.end(), Material);
	if (found == m_vMaterialFrictionData.end())
	{
		JW_ERROR_ABORT("Cannot find mathcing material.");
	}
	else
	{
		return *found;
	}
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
	// Collision #1
	DetectCoarseCollision();

	// Collision #2
	DetectFineCollision();

	// Collision #3
	ProcessCollision();

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

				// FOR DEBUGGING ?????
				delta_time = min(delta_time, 0.1f);

				// a = 1/m * f
				iter.Acceleration = iter.InverseMass * iter.AccumulatedForce;

				// v' = v + at
				iter.Velocity += iter.Acceleration * delta_time;

				/// dv = 1/m * g
				///iter.Velocity += iter.InverseMass * iter.AccumulatedImpulse;

				// apply damping
				iter.Velocity *= powf(iter.Damping, delta_time);

				// killing micro-collisions ...?
				auto velocity_x = XMVectorGetX(iter.Velocity);
				auto velocity_y = XMVectorGetY(iter.Velocity);
				auto velocity_z = XMVectorGetZ(iter.Velocity);
				if (fabsf(velocity_x) <= 0.02f)
				{
					XMVectorSetX(iter.Velocity, 0.0f);
				}
				if (fabsf(velocity_y) <= 0.02f)
				{
					XMVectorSetY(iter.Velocity, 0.0f);
				}
				if (fabsf(velocity_z) <= 0.02f)
				{
					XMVectorSetZ(iter.Velocity, 0.0f);
				}

				// meet my world floor
				if (XMVectorGetY(transform->Position) < KPhysicsWorldFloor)
				{
					iter.Velocity = KVectorZero;
				}

				// p' = p + vt
				transform->Position += iter.Velocity * delta_time;

				// DEBUGGING
				/*
				std::cout
					<< "Position = { " 
					<< TO_STRING(XMVectorGetX(transform->Position)) << " , "
					<< TO_STRING(XMVectorGetY(transform->Position)) << " , "
					<< TO_STRING(XMVectorGetZ(transform->Position)) << " }"
					<< std::endl;
				*/

				// Clear accumulations
				iter.ClearAccumulation();
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
	return a.Distance < b.Distance;
}

bool ClosestFacePred(const SClosestFace& a, const SClosestFace& b)
{
	return fabsf(a.Dist) < fabsf(b.Dist);
}

PRIVATE void JWSystemPhysics::DetectFineCollision() noexcept
{
	// Early out
	if (m_CoarseCollisionList.size() == 0) { return; }

	m_FineCollisionList.clear();
	m_IsThereAnyActualCollision = false;

	for (const auto& iter : m_CoarseCollisionList)
	{
		const auto& a_physics = m_vComponents[iter.A];
		const auto& b_physics = m_vComponents[iter.B];

		auto& a_collision_mesh{ a_physics.PtrCollisionMesh };
		auto& b_collision_mesh{ b_physics.PtrCollisionMesh };
		if ((a_collision_mesh == nullptr) || (b_collision_mesh == nullptr))
		{
			JW_ERROR_ABORT("Collision mesh is missing.");
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


		// #3 Transform objects' faces into world space
		// #3-1 Transform object a's faces into world space
		m_TransformedFacesA.clear();
		m_TransformedFacesA.reserve(a_faces.size());
		for (const auto& a_face : a_faces)
		{
			auto a0 = XMVector3TransformCoord(a_positions[a_v_to_pv[a_face._0]], a_transform->WorldMatrix);
			auto a1 = XMVector3TransformCoord(a_positions[a_v_to_pv[a_face._1]], a_transform->WorldMatrix);
			auto a2 = XMVector3TransformCoord(a_positions[a_v_to_pv[a_face._2]], a_transform->WorldMatrix);
			auto n = GetTriangleNormal(a0, a1, a2);
			auto m = (a0 + a1 + a2) / 3.0f;

			m_TransformedFacesA.emplace_back(a0, a1, a2, n, m);
		}

		// #3-2 Transform object b's faces into world space
		m_TransformedFacesB.clear();
		m_TransformedFacesB.reserve(b_faces.size());
		for (const auto& b_face : b_faces)
		{
			auto b0 = XMVector3TransformCoord(b_positions[b_v_to_pv[b_face._0]], b_transform->WorldMatrix);
			auto b1 = XMVector3TransformCoord(b_positions[b_v_to_pv[b_face._1]], b_transform->WorldMatrix);
			auto b2 = XMVector3TransformCoord(b_positions[b_v_to_pv[b_face._2]], b_transform->WorldMatrix);
			auto n = GetTriangleNormal(b0, b1, b2);
			auto m = (b0 + b1 + b2) / 3.0f;

			m_TransformedFacesB.emplace_back(b0, b1, b2, n, m);
		}

		// #4 Find the closest faces of each object to another
		// #4-1 See if any plane of object a has projected center point of object b
		m_ClosestFacesA.clear();
		m_ClosestFacesA.reserve(m_TransformedFacesA.size());
		float dist{};
		for (const auto& a_face : m_TransformedFacesA)
		{
			auto projected = ProjectPointOntoPlane(dist, b_center_world, a_face.V0, a_face.N);
			if (IsPointOnPlaneInsideTriangle(projected, a_face.V0, a_face.V1, a_face.V2))
			{
				m_ClosestFacesA.emplace_back(dist, a_face.V0, a_face.V1, a_face.V2, a_face.N, projected);
			}
		}

		// #4-2 See if any plane of object b has projected center point of object a
		m_ClosestFacesB.clear();
		m_ClosestFacesB.reserve(m_TransformedFacesB.size());
		for (const auto& b_face : m_TransformedFacesB)
		{
			auto projected = ProjectPointOntoPlane(dist, a_center_world, b_face.V0, b_face.N);
			if (IsPointOnPlaneInsideTriangle(projected, b_face.V0, b_face.V1, b_face.V2))
			{
				m_ClosestFacesB.emplace_back(dist, b_face.V0, b_face.V1, b_face.V2, b_face.N, projected);
			}
		}

		if ((m_ClosestFacesA.size() == 0) && (m_ClosestFacesB.size() == 0))
		{
			// Early out (not intersecting)
			continue;
		}
		
		if (m_ClosestFacesA.size())
		{
			// object a is bigger than object b

			std::sort(m_ClosestFacesA.begin(), m_ClosestFacesA.end(), ClosestFacePred);
			m_ClosestFaceA = m_ClosestFacesA.front();

			m_ClosestFacesB.clear();
			for (const auto& b_face : m_TransformedFacesB)
			{
				auto sq_dist = XMVectorGetX(XMVector3LengthSq(m_ClosestFaceA.Projected - b_face.M));
				
				m_ClosestFacesB.emplace_back(sq_dist, b_face.V0, b_face.V1, b_face.V2, b_face.N, b_face.M);
			}
			std::sort(m_ClosestFacesB.begin(), m_ClosestFacesB.end(), ClosestFacePred);
			m_ClosestFaceB = m_ClosestFacesB.front();

		}
		else
		{
			// object b is bigger than object a

			std::sort(m_ClosestFacesB.begin(), m_ClosestFacesB.end(), ClosestFacePred);
			m_ClosestFaceB = m_ClosestFacesB.front();

			m_ClosestFacesA.clear();
			for (const auto& a_face : m_TransformedFacesA)
			{
				auto sq_dist = XMVectorGetX(XMVector3LengthSq(m_ClosestFaceB.Projected - a_face.M));

				m_ClosestFacesA.emplace_back(sq_dist, a_face.V0, a_face.V1, a_face.V2, a_face.N, a_face.M);
			}
			std::sort(m_ClosestFacesA.begin(), m_ClosestFacesA.end(), ClosestFacePred);
			m_ClosestFaceA = m_ClosestFacesA.front();
		}


		// #5 Get the closest points of each object to another
		// #5-1 Get closest points of object a to object b's center
		m_ClosestPointsA.clear();
		m_ClosestPointsA.resize(3);
		m_ClosestPointsA[0].Point = m_ClosestFaceA.V0;
		m_ClosestPointsA[1].Point = m_ClosestFaceA.V1;
		m_ClosestPointsA[2].Point = m_ClosestFaceA.V2;

		for (auto& a_point : m_ClosestPointsA)
		{
			a_point.Distance = XMVectorGetX(XMVector3LengthSq(b_center_world - a_point.Point));
		}
		std::sort(m_ClosestPointsA.begin(), m_ClosestPointsA.end(), ClosestPointPred);
		m_ClosestPointA = m_ClosestPointsA.front();

		// #5-2 Get closest points of object b to object a's center
		m_ClosestPointsB.clear();
		m_ClosestPointsB.resize(3);
		m_ClosestPointsB[0].Point = m_ClosestFaceB.V0;
		m_ClosestPointsB[1].Point = m_ClosestFaceB.V1;
		m_ClosestPointsB[2].Point = m_ClosestFaceB.V2;

		for (auto& b_point : m_ClosestPointsB)
		{
			b_point.Distance = XMVectorGetX(XMVector3LengthSq(b_center_world - b_point.Point));
		}
		std::sort(m_ClosestPointsB.begin(), m_ClosestPointsB.end(), ClosestPointPred);
		m_ClosestPointB = m_ClosestPointsB.front();


		// #6 See if the two objects intersect
		ECollisionType collision_type{ ECollisionType::None };

		// #6-1 Point - Face
		// #6-1-1 Point of a - Face of b
		{
			const auto& p_a = m_ClosestPointA.Point;
			float dist_ab{};
			ProjectPointOntoPlane(dist_ab, p_a, m_ClosestPointB.Point, ba_dir);
			if (dist_ab < 0)
			{
				if (IsPointAInB(p_a, b_faces, b_transform->WorldMatrix, b_v_to_pv, b_positions))
				{
					collision_type = ECollisionType::PointAFaceB;
				}
			}
		}

		// #6-1-2 Point of b - Face of a
		if (collision_type == ECollisionType::None)
		{
			const auto& p_b = m_ClosestPointB.Point;
			float dist_ba{};
			ProjectPointOntoPlane(dist_ba, p_b, m_ClosestPointA.Point, ab_dir);
			if (dist_ba < 0)
			{
				if (IsPointAInB(p_b, a_faces, a_transform->WorldMatrix, a_v_to_pv, a_positions))
				{
					collision_type = ECollisionType::PointBFaceA;
				}
			}
		}
		
		// #6-2 Edge - Edge
		SClosestEdge edge_a{};
		SClosestEdge edge_b{};
		EClosestEdgePair edge_pair_a{ EClosestEdgePair::None };
		EClosestEdgePair edge_pair_b{ EClosestEdgePair::None };

		// #6-2-1 Edge of 'b' is intersecting the closest face of 'a'
		// Find the edge of 'a' that intersects 'b'
		if (collision_type == ECollisionType::None)
		{
			XMVECTOR closest_point_of_edge_a_to_b{};

			// Edge a V0-V1
			if (ProjectPointOntoSegment(b_center_world, m_ClosestFaceA.V0, m_ClosestFaceA.V1, closest_point_of_edge_a_to_b))
			{
				if (IsPointAInB(closest_point_of_edge_a_to_b, b_faces, b_transform->WorldMatrix, b_v_to_pv, b_positions))
				{
					edge_pair_a = EClosestEdgePair::V0V1;
				}
			}

			// Edge a V0-V2
			if (edge_pair_a == EClosestEdgePair::None)
			{
				if (ProjectPointOntoSegment(b_center_world, m_ClosestFaceA.V0, m_ClosestFaceA.V2, closest_point_of_edge_a_to_b))
				{
					if (IsPointAInB(closest_point_of_edge_a_to_b, b_faces, b_transform->WorldMatrix, b_v_to_pv, b_positions))
					{
						edge_pair_a = EClosestEdgePair::V0V2;
					}
				}
			}

			// Edge a V1-V2
			if (edge_pair_a == EClosestEdgePair::None)
			{
				if (ProjectPointOntoSegment(b_center_world, m_ClosestFaceA.V1, m_ClosestFaceA.V2, closest_point_of_edge_a_to_b))
				{
					if (IsPointAInB(closest_point_of_edge_a_to_b, b_faces, b_transform->WorldMatrix, b_v_to_pv, b_positions))
					{
						edge_pair_a = EClosestEdgePair::V1V2;
					}
				}
			}

			// Find the edge of 'b' that intersects the closest face of 'a'
			if (edge_pair_a != EClosestEdgePair::None)
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
					edge_pair_b = EClosestEdgePair::V0V1;
				}

				// Edge b V0-V2
				if (edge_pair_b == EClosestEdgePair::None)
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
						edge_pair_b = EClosestEdgePair::V0V2;
					}
				}

				// Edge b V1-V2
				if (edge_pair_b == EClosestEdgePair::None)
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
						edge_pair_b = EClosestEdgePair::V1V2;
					}
				}
			}

			if (edge_pair_a != EClosestEdgePair::None)
			{
				// If there is edge_a, there must be edge_b too.
				if (edge_pair_b == EClosestEdgePair::None)
				{
					// To prevent robustness error,
					// arbitrarily assign an edge.
					edge_pair_b = EClosestEdgePair::V0V1;
				}

				switch (edge_pair_a)
				{
				case JWEngine::EClosestEdgePair::V0V1:
					edge_a = SClosestEdge(m_ClosestFaceA.V0, m_ClosestFaceA.V1);
					break;
				case JWEngine::EClosestEdgePair::V0V2:
					edge_a = SClosestEdge(m_ClosestFaceA.V0, m_ClosestFaceA.V2);
					break;
				case JWEngine::EClosestEdgePair::V1V2:
					edge_a = SClosestEdge(m_ClosestFaceA.V1, m_ClosestFaceA.V2);
					break;
				}
				edge_a.M = (edge_a.V0 + edge_a.V1) / 2.0f;

				switch (edge_pair_b)
				{
				case JWEngine::EClosestEdgePair::V0V1:
					edge_b = SClosestEdge(m_ClosestFaceB.V0, m_ClosestFaceB.V1);
					break;
				case JWEngine::EClosestEdgePair::V0V2:
					edge_b = SClosestEdge(m_ClosestFaceB.V0, m_ClosestFaceB.V2);
					break;
				case JWEngine::EClosestEdgePair::V1V2:
					edge_b = SClosestEdge(m_ClosestFaceB.V1, m_ClosestFaceB.V2);
					break;
				}
				edge_b.M = (edge_b.V0 + edge_b.V1) / 2.0f;

				collision_type = ECollisionType::EdgeEdge;
			}
		}

		// #6-2-2 Edge of 'a' is intersecting the closest face of 'b'
		// Find the edge of 'b' that intersects 'a'
		if (collision_type == ECollisionType::None)
		{
			XMVECTOR closest_point_of_edge_b_to_a{};

			// Edge b V0-V1
			if (ProjectPointOntoSegment(a_center_world, m_ClosestFaceB.V0, m_ClosestFaceB.V1, closest_point_of_edge_b_to_a))
			{
				if (IsPointAInB(closest_point_of_edge_b_to_a, a_faces, a_transform->WorldMatrix, a_v_to_pv, a_positions))
				{
					edge_pair_b = EClosestEdgePair::V0V1;
				}
			}

			// Edge b V0-V2
			if (edge_pair_b == EClosestEdgePair::None)
			{
				if (ProjectPointOntoSegment(a_center_world, m_ClosestFaceB.V0, m_ClosestFaceB.V2, closest_point_of_edge_b_to_a))
				{
					if (IsPointAInB(closest_point_of_edge_b_to_a, a_faces, a_transform->WorldMatrix, a_v_to_pv, a_positions))
					{
						edge_pair_b = EClosestEdgePair::V0V2;
					}
				}
			}

			// Edge b V1-V2
			if (edge_pair_b == EClosestEdgePair::None)
			{
				if (ProjectPointOntoSegment(a_center_world, m_ClosestFaceB.V1, m_ClosestFaceB.V2, closest_point_of_edge_b_to_a))
				{
					if (IsPointAInB(closest_point_of_edge_b_to_a, a_faces, a_transform->WorldMatrix, a_v_to_pv, a_positions))
					{
						edge_pair_b = EClosestEdgePair::V1V2;
					}
				}
			}

			// Find the edge of 'a' that intersects the closest face of 'b'
			if (edge_pair_b != EClosestEdgePair::None)
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
					edge_pair_a = EClosestEdgePair::V0V1;
				}

				// Edge a V0-V2
				if (edge_pair_a == EClosestEdgePair::None)
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
						edge_pair_a = EClosestEdgePair::V0V2;
					}
				}

				// Edge a V1-V2
				if (edge_pair_a == EClosestEdgePair::None)
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
						edge_pair_a = EClosestEdgePair::V1V2;
					}
				}
			}

			if (edge_pair_b != EClosestEdgePair::None)
			{
				// If there is edge_b, there must be edge_a too.
				if (edge_pair_a == EClosestEdgePair::None)
				{
					// To prevent robustness error,
					// arbitrarily assign an edge.
					edge_pair_a = EClosestEdgePair::V0V1;
				}

				collision_type = ECollisionType::EdgeEdge;
			}
		}

		if (collision_type != ECollisionType::None)
		{
			m_IsThereAnyActualCollision = true;

			// #7 Get signed closing speed
			auto relative_velocity_ba = b_physics.Velocity - a_physics.Velocity;
			auto signed_closing_speed = XMVectorGetX(XMVector3Dot(relative_velocity_ba, ba_dir));
			
			// #8 Get collision data (collision normal & penetration depth)
			const auto& a_entity = m_pECS->GetEntityByIndex(a_physics.EntityIndex);
			const auto& b_entity = m_pECS->GetEntityByIndex(b_physics.EntityIndex);
			XMVECTOR collision_normal{};
			float penetration_depth{};
				
			switch (collision_type)
			{
			case JWEngine::ECollisionType::PointAFaceB:
				ProjectPointOntoPlane(penetration_depth, m_ClosestPointA.Point, m_ClosestFaceB.V0, m_ClosestFaceB.N);
				if (penetration_depth < 0) { penetration_depth = -penetration_depth; };

				// Collision normal : b to a
				collision_normal = m_ClosestFaceB.N;

				// DEBUGGING
				//std::cout << "[Collision] Point A in Face B" << std::endl;

				break;
			case JWEngine::ECollisionType::PointBFaceA:
				ProjectPointOntoPlane(penetration_depth, m_ClosestPointB.Point, m_ClosestFaceA.V0, m_ClosestFaceA.N);
				if (penetration_depth < 0) { penetration_depth = -penetration_depth; };

				// Collision normal : b to a
				collision_normal = -m_ClosestFaceA.N;

				// DEBUGGING
				//std::cout << "[Collision] Point B in Face A" << std::endl;

				break;
			case JWEngine::ECollisionType::EdgeEdge:
				penetration_depth = XMVectorGetX(XMVector3Length(edge_a.M - edge_b.M));

				collision_normal = XMVector3Normalize(
					XMVector3Cross(GetRayDirection(edge_b.V0, edge_b.V1), GetRayDirection(edge_a.V0, edge_a.V1)));
					
				// Collision normal : b to a
				if (XMVector3Less(XMVector3Dot(collision_normal, ba_dir), KVectorZero))
				{
					collision_normal = -collision_normal;
				}
				
				// DEBUGGING
				//std::cout << "[Collision] Edge to Edge" << std::endl;

				break;
			}
				
			m_FineCollisionList.emplace_back(a_entity, b_entity, ba_dir, collision_normal, penetration_depth, signed_closing_speed);
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

void JWSystemPhysics::ProcessCollision() noexcept
{
	// Early out
	if (m_FineCollisionList.size() == 0) { return; }

	for (const auto& iter : m_FineCollisionList)
	{
		auto a_physics{ iter.PtrEntityA->GetComponentPhysics() };
		auto a_collision_speed = XMVector3Dot(a_physics->Velocity, -iter.CollisionNormal);
		auto a_collision_velocity = a_collision_speed * -iter.CollisionNormal;
		auto a_separating_speed = a_collision_speed * (1.0f + a_physics->Restitution);
		auto a_delta_velocity = a_separating_speed * iter.CollisionNormal;
		
		auto b_physics{ iter.PtrEntityB->GetComponentPhysics() };
		auto b_collision_speed = XMVector3Dot(b_physics->Velocity, iter.CollisionNormal);
		auto b_collision_velocity = b_collision_speed * iter.CollisionNormal;
		auto b_separating_speed = b_collision_speed * (1.0f + b_physics->Restitution);
		auto b_delta_velocity = b_separating_speed * -iter.CollisionNormal;

		float a_mass{ 0.0f };
		float b_mass{ 0.0f };

		if (a_physics->InverseMass != 0)
		{
			a_mass = 1.0f / a_physics->InverseMass;
		}

		if (b_physics->InverseMass != 0)
		{
			b_mass = 1.0f / b_physics->InverseMass;
		}

		// Resolve penetration
		auto a_transform{ iter.PtrEntityA->GetComponentTransform() };
		auto a_penetration_resolution{ (a_mass / (a_mass + b_mass)) * iter.PenetrationDepth * iter.CollisionNormal };
		a_transform->Position += a_penetration_resolution;

		auto b_transform{ iter.PtrEntityB->GetComponentTransform() };
		auto b_penetration_resolution{ (b_mass / (a_mass + b_mass)) * iter.PenetrationDepth * -iter.CollisionNormal };
		b_transform->Position += b_penetration_resolution;

		auto a_sliding_velocity = a_physics->Velocity - a_collision_velocity;
		auto b_sliding_velocity = b_physics->Velocity - b_collision_velocity;

		//auto static_friction{ a_physics->MaterialFriction.StaticFrictionConstant * b_physics->MaterialFriction.StaticFrictionConstant };
		auto kinetic_friction{ a_physics->MaterialFriction.KineticFrictionConstant * b_physics->MaterialFriction.KineticFrictionConstant };

		auto a_friction_size = XMVectorGetX(XMVector3Length(a_collision_velocity * kinetic_friction));
		auto a_friction_velocity = XMVector3Normalize(-a_sliding_velocity) * a_friction_size;

		auto b_friction_size = XMVectorGetX(XMVector3Length(b_collision_velocity * kinetic_friction));
		auto b_friction_velocity = XMVector3Normalize(-b_sliding_velocity) * b_friction_size;

		if (a_physics->InverseMass != 0)
		{
			a_physics->Velocity += a_delta_velocity + a_friction_velocity;
		}

		if (b_physics->InverseMass != 0)
		{
			b_physics->Velocity += b_delta_velocity + b_friction_velocity;
		}

		// DEBUGGING
		/*
		std::cout
			<< "Velocity = { "
			<< TO_STRING(XMVectorGetX(b_physics->Velocity)) << " , "
			<< TO_STRING(XMVectorGetY(b_physics->Velocity)) << " , "
			<< TO_STRING(XMVectorGetZ(b_physics->Velocity)) << " }"
			<< std::endl;
		*/
	}
}
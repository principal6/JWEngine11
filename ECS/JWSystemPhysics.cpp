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

void JWSystemPhysics::Destroy() noexcept
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}
}

auto JWSystemPhysics::CreateComponent(JWEntity* pEntity) noexcept->SComponentPhysics&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };

	auto new_entry{ new SComponentPhysics() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID & pointer to Entity
	m_vpComponents[slot]->ComponentID = slot;
	m_vpComponents[slot]->PtrEntity = pEntity;

	// Set world matrix of bounding ellipsoid
	auto transform = m_vpComponents[slot]->PtrEntity->GetComponentTransform();
	const auto& bounding_ellipsoid = m_vpComponents[slot]->BoundingEllipsoid;

	auto temp_center = bounding_ellipsoid.Offset;
	if (transform) { temp_center += transform->Position; }
	
	auto mat_scaling = XMMatrixScaling(bounding_ellipsoid.RadiusX, bounding_ellipsoid.RadiusY, bounding_ellipsoid.RadiusZ);
	auto mat_translation = XMMatrixTranslationFromVector(temp_center);

	// Add bounding ellipsoid instance into SystemRender
	m_pECS->SystemRender().AddBoundingEllipsoidInstance(mat_scaling * mat_translation);

	return *m_vpComponents[slot];
}

void JWSystemPhysics::DestroyComponent(SComponentPhysics& Component) noexcept
{
	// Erase bounding ellipsoid instance in SystemRender
	m_pECS->SystemRender().EraseBoundingEllipsoidInstance(Component.ComponentID);

	uint32_t slot{};
	for (const auto& iter : m_vpComponents)
	{
		if (iter->ComponentID == Component.ComponentID)
		{
			break;
		}

		++slot;
	}
	JW_DELETE(m_vpComponents[slot]);

	// Swap the last element of the vector and the deleted element & shrink the size of the vector.
	uint32_t last_index = static_cast<uint32_t>(m_vpComponents.size() - 1);
	if (slot < last_index)
	{
		m_vpComponents[slot] = m_vpComponents[last_index];
		m_vpComponents[slot]->ComponentID = slot; // @important!!

		m_vpComponents[last_index] = nullptr;
	}

	m_vpComponents.pop_back();
}

auto JWSystemPhysics::PickEntity() noexcept->bool
{
	m_pPickedEntity = nullptr;
	m_pPickedTerrainEntity = nullptr;
	m_pPickedNonTerrainEntity = nullptr;
	
	CastPickingRay();

	if (m_vpComponents.size() == 0)
	{
		return false;
	}

	// Reset distances for comparison
	m_PickedTerrainDistance = KVectorMax;
	m_PickedNonTerrainDistance = KVectorMax;

	if (PickEntityByEllipsoid())
	{
		if (m_pPickedTerrainEntity)
		{
			if (PickSubBoundingEllipsoid(m_pPickedTerrainEntity))
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

PRIVATE auto JWSystemPhysics::PickEntityByEllipsoid() noexcept->bool
{
	for (auto iter : m_vpComponents)
	{
		auto entity = iter->PtrEntity;

		auto transform{ entity->GetComponentTransform() };
		auto type{ entity->GetEntityType() };

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
			auto camera{ entity->GetComponentCamera() };
			if (camera)
			{
				// @important
				// You must be UNABLE to pick the current camera!
				if (m_pECS->SystemCamera().GetCurrentCameraComponentID() == camera->ComponentID)
				{
					continue;
				}
			}

			// Transform ray into ellipsoid space
			auto inv_ellipsoid_world = XMMatrixInverse(nullptr, iter->BoundingEllipsoid.EllipsoidWorld);
			auto e_ray_origin = XMVector3TransformCoord(m_PickingRayOrigin, inv_ellipsoid_world);
			auto e_ray_direction = XMVector3TransformNormal(m_PickingRayDirection, inv_ellipsoid_world);

			if (type == EEntityType::MainTerrain)
			{
				if (IntersectRayUnitSphere(e_ray_origin, e_ray_direction))
				{
					m_pPickedTerrainEntity = entity;
				}
			}
			else
			{
				if (IntersectRayUnitSphere(e_ray_origin, e_ray_direction, &m_PickedNonTerrainDistance))
				{
					m_pPickedNonTerrainEntity = entity;
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

		for (auto& bs : m_vPickedSubBoundingEllipsoidID)
		{
			for (auto& node : ptr_terrain->QuadTree)
			{
				if (node.SubBoundingEllipsoidID == bs)
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
	for (auto& iter : m_vpComponents)
	{
		if (iter->InverseMass > 0)
		{
			iter->AccumulatedForce += _Acceleration / iter->InverseMass;
		}
	}
}

void JWSystemPhysics::ZeroAllVelocities() noexcept
{
	for (auto& iter : m_vpComponents)
	{
		if (iter->InverseMass > 0)
		{
			iter->Velocity = KVectorZero;
		}
	}
}

void JWSystemPhysics::Execute() noexcept
{
	for (auto& iter : m_vpComponents)
	{
		auto transform{ iter->PtrEntity->GetComponentTransform() };
		if (transform)
		{
			if (iter->InverseMass > 0)
			{
				auto delta_time = m_pECS->GetDeltaTime();
				assert(delta_time >= 0);

				// a = 1/m * f;
				iter->Acceleration = iter->InverseMass * iter->AccumulatedForce;

				// v' = v + at
				iter->Velocity += iter->Acceleration * delta_time;

				// p' = p + vt
				transform->Position += iter->Velocity * delta_time;

				// Reset accumulated force
				iter->AccumulatedForce = KVectorZero;
			}

			// Update bounding ellipsoid
			UpdateBoundingEllipsoid(*iter);

			// Calculate world matrix of the sub-bounding ellipsoids
			auto entity_position = transform->Position;
			for (auto& curr_sub_be : iter->SubBoundingEllipsoids)
			{
				auto mat_scaling = XMMatrixScaling(curr_sub_be.RadiusX, curr_sub_be.RadiusY, curr_sub_be.RadiusZ);
				auto mat_translation = XMMatrixTranslationFromVector(entity_position + curr_sub_be.Offset);
				curr_sub_be.EllipsoidWorld = mat_scaling * mat_translation;
			}
		}
	}
}

PRIVATE void JWSystemPhysics::UpdateBoundingEllipsoid(SComponentPhysics& Physics) noexcept
{
	// Calculate world matrix of the bounding ellipsoid
	auto transform = Physics.PtrEntity->GetComponentTransform();
	auto& bounding_ellipsoid = Physics.BoundingEllipsoid;

	auto temp_center = bounding_ellipsoid.Offset;
	if (transform) { temp_center += transform->Position; }

	auto mat_scaling = XMMatrixScaling(bounding_ellipsoid.RadiusX, bounding_ellipsoid.RadiusY, bounding_ellipsoid.RadiusZ);
	auto mat_translation = XMMatrixTranslationFromVector(temp_center);
	bounding_ellipsoid.EllipsoidWorld = mat_scaling * mat_translation;

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingEllipsoidInstance(Physics.ComponentID, bounding_ellipsoid.EllipsoidWorld);
}
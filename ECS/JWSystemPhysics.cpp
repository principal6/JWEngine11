#include "JWECS.h"

using namespace JWEngine;

void JWSystemPhysics::Create(JWECS& ECS, HWND hWnd, XMFLOAT2 WindowSize) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;

	m_hWnd = hWnd;
	m_WindowSize = WindowSize;
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

	// Set center position
	auto transform = m_vpComponents[slot]->PtrEntity->GetComponentTransform();
	if (transform)
	{
		m_vpComponents[slot]->BoundingSphereCenterPosition = m_vpComponents[slot]->BoundingSphereCenterOffset + transform->Position;
	}

	// Add bounding volume instance into SystemRender
	m_pECS->SystemRender().AddBoundingVolumeInstance(m_vpComponents[slot]->BoundingSphereRadius, m_vpComponents[slot]->BoundingSphereCenterPosition);

	return *m_vpComponents[slot];
}

void JWSystemPhysics::DestroyComponent(SComponentPhysics& Component) noexcept
{
	// Erase bounding volume instance in SystemRender
	m_pECS->SystemRender().EraseBoundingVolumeInstance(Component.ComponentID);

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
		m_vpComponents[last_index] = nullptr;
	}

	m_vpComponents.pop_back();
}

void JWSystemPhysics::SetBoundingSphere(JWEntity* pEntity, float Radius, const XMFLOAT3& CenterOffset) noexcept
{
	if (pEntity == nullptr) { return; }
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Set the radius
	physics->BoundingSphereRadius = Radius;

	// Set center position
	auto transform = physics->PtrEntity->GetComponentTransform();
	physics->BoundingSphereCenterOffset = XMLoadFloat3(&CenterOffset);
	physics->BoundingSphereCenterPosition = physics->BoundingSphereCenterOffset + transform->Position;
	
	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingVolumeInstance(physics->ComponentID, physics->BoundingSphereRadius, physics->BoundingSphereCenterPosition);
}

void JWSystemPhysics::HideBoundingSphere(JWEntity* pEntity) noexcept
{
	if (pEntity == nullptr) { return; }
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingVolumeInstance(physics->ComponentID, 0, XMVectorZero());
}

void JWSystemPhysics::UnhideBoundingSphere(JWEntity* pEntity) noexcept
{
	if (pEntity == nullptr) { return; }
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingVolumeInstance(physics->ComponentID, physics->BoundingSphereRadius, physics->BoundingSphereCenterPosition);
}

void JWSystemPhysics::UpdateBoundingSphere(JWEntity* pEntity) noexcept
{
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Set center position
	auto transform = physics->PtrEntity->GetComponentTransform();
	if (transform)
	{
		physics->BoundingSphereCenterPosition = physics->BoundingSphereCenterOffset + transform->Position;
	}

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingVolumeInstance(physics->ComponentID, physics->BoundingSphereRadius, physics->BoundingSphereCenterPosition);
}

void JWSystemPhysics::Pick() noexcept
{
	CastPickingRay();

	//PickEntityByTriangle();
	PickEntityBySphere();
}

PRIVATE void JWSystemPhysics::CastPickingRay() noexcept
{
	// Get mouse cursor position in screen space (in client rect)
	GetCursorPos(&m_MouseClientPosition);
	ScreenToClient(m_hWnd, &m_MouseClientPosition);

	// Normalize mouse cursor position
	m_NormalizedMousePosition.x = (static_cast<float>(m_MouseClientPosition.x) / m_WindowSize.x) * 2.0f - 1.0f;
	m_NormalizedMousePosition.y = (static_cast<float>(m_MouseClientPosition.y) / m_WindowSize.y) * 2.0f - 1.0f;

	auto MatrixView = m_pECS->SystemCamera().CurrentViewMatrix();
	auto MatrixProjection = m_pECS->SystemCamera().CurrentProjectionMatrix();

	m_PickingRayViewSpacePosition = XMVectorSet(0, 0, 0.001f, 0);
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

PRIVATE void JWSystemPhysics::PickEntityByTriangle() noexcept
{
	m_PickedEntityName.clear();

	if (m_vpComponents.size())
	{
		XMVECTOR t_cmp{ XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 0) };

		for (auto iter : m_vpComponents)
		{
			auto entity = iter->PtrEntity;

			auto transform{ entity->GetComponentTransform() };
			auto render{ entity->GetComponentRender() };
			auto type{ entity->GetEntityType() };

			if (type != EEntityType::UserDefined)
			{
				continue;
			}

			if (transform)
			{
				auto model_type = render->PtrModel->GetRenderType();
				XMVECTOR t{};

				SIndexDataTriangle* ptr_index_data{};
				SVertexDataModel* ptr_vertex_data{};

				if ((model_type == ERenderType::Model_Dynamic) || (model_type == ERenderType::Model_Static))
				{
					ptr_index_data = &render->PtrModel->ModelData.IndexData;
					ptr_vertex_data = &render->PtrModel->ModelData.VertexData;
				}
				else if (model_type == ERenderType::Model_Rigged)
				{
					ptr_index_data = &render->PtrModel->ModelData.IndexData;
					ptr_vertex_data = &render->PtrModel->ModelData.VertexData;
				}
				else
				{
					continue;
				}

				assert(ptr_index_data);
				assert(ptr_vertex_data);

				const auto& indices{ ptr_index_data->vIndices };
				const auto& vertices{ ptr_vertex_data->vVerticesModel };

				// Iterate all the triangles in the model
				for (auto triangle : indices)
				{
					auto v0 = XMLoadFloat3(&vertices[triangle._0].Position);
					auto v1 = XMLoadFloat3(&vertices[triangle._1].Position);
					auto v2 = XMLoadFloat3(&vertices[triangle._2].Position);

					// Move vertices from local space to world space!
					v0 = XMVector3TransformCoord(v0, transform->WorldMatrix);
					v1 = XMVector3TransformCoord(v1, transform->WorldMatrix);
					v2 = XMVector3TransformCoord(v2, transform->WorldMatrix);

					if (PickTriangle(v0, v1, v2, t_cmp))
					{
						m_pPickedEntity = entity;
					}
				}
			}
		}

		if (m_pPickedEntity)
		{
			m_PickedEntityName = m_pPickedEntity->GetEntityName();
		}
	}
}

PRIVATE __forceinline auto JWSystemPhysics::PickTriangle(XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2, XMVECTOR& t_cmp) noexcept->bool
{
	// Calculate edge vectors from vertex positions
	auto edge0 = V1 - V0;
	auto edge1 = V2 - V0;

	// Calculate face normal from edge vectors,
	// using cross product of vectors
	auto normal = XMVector3Normalize(XMVector3Cross(edge0, edge1));

	// Calculate plane equation  # Ax + By + Cz + D = 0
	// # A, B, C = Face normal's xyz coord
	// # x, y, z = Any point in the plane, so we can just use V0
	// # Ax + By + Cz = Dot(normal, point) = Dot(N, P)
	// # D = -(Ax + By + Cz) = -Dot(normal, v0) = -Dot(N, V0)
	//
	// @ Plane equation for a point P
	// Dot(N, P) = Dot(N, V0)
	auto D = -XMVector3Dot(normal, V0);

	// Get ray's equation (which is a parametric equation of a line)
	//
	// L = P0 + t * P1
	//   = ray_origin + t * ray_direction
	//
	// @ N: plane normal  @ V = given vertex in the plane  @ L = line vector
	//
	// L = (P0x + P1x * t, P0y + P1y * t, P0z + P1z * t)
	//
	// Dot(N, L) = Dot(N, V)
	// => (P0x + P1x * t) * Nx + (P0y + P1y * t) * Ny + (P0z + P1z * t) * Nz = (Vx) * Nx + (Vy) * Ny + (Vz) * Nz
	// => (P1x * t) * Nx + (P1y * t) * Ny + (P1z * t) * Nz = (Vx - P0x) * Nx + (Vy - P0y) * Ny + (Vz - P0z) * Nz
	// => (P1x * Nx) * t + (P1y * Ny) * t + (P1z * Nz) * t = (Vx - P0x) * Nx + (Vy - P0y) * Ny + (Vz - P0z) * Nz
	// => Dot(P1, N) * t = Dot(V, N) - Dot(P0, N)
	//
	//           Dot(V, N) - Dot(P0, N)
	// =>  t  = ------------------------
	//                 Dot(P1, N)
	//
	auto p0_norm = XMVector3Dot(m_PickingRayOrigin, normal);
	auto p1_norm = XMVector3Dot(m_PickingRayDirection, normal);
	XMVECTOR zero{ XMVectorZero() };
	XMVECTOR t{};

	// For vectorization we use vectors to compare values
	if (XMVector3NotEqual(p1_norm, zero))
	{
		t = (-D - p0_norm) / p1_norm;
	}

	// 't' should be positive for the picking to be in front of the camera!
	// (if it's negative, the picking is occuring behind the camera)
	// We will store the minimum of t values, which means that it's the closest picking to the camera.
	if ((XMVector3Greater(t, zero)) && (XMVector3Less(t, t_cmp)))
	{
		auto point_on_plane = m_PickingRayOrigin + t * m_PickingRayDirection;

		// Check if the point is in the triangle
		if (IsPointInTriangle(point_on_plane, V0, V1, V2))
		{
			m_PickedTriangle[0] = V0;
			m_PickedTriangle[1] = V1;
			m_PickedTriangle[2] = V2;

			t_cmp = t;
			
			return true;
		}
	}

	return false;
}

PRIVATE __forceinline auto JWSystemPhysics::IsPointInTriangle(XMVECTOR& Point, XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2) noexcept->bool
{
	bool result{ false };
	auto zero = XMVectorZero();
	auto check_0 = XMVector3Cross((V2 - V1), (Point - V1));
	auto check_1 = XMVector3Cross((V2 - V1), (V0 - V1));

	if (XMVector3Greater(XMVector3Dot(check_0, check_1), zero))
	{
		check_0 = XMVector3Cross((V2 - V0), (Point - V0));
		check_1 = XMVector3Cross((V2 - V0), (V1 - V0));

		if (XMVector3Greater(XMVector3Dot(check_0, check_1), zero))
		{
			check_0 = XMVector3Cross((V1 - V0), (Point - V0));
			check_1 = XMVector3Cross((V1 - V0), (V2 - V0));

			if (XMVector3Greater(XMVector3Dot(check_0, check_1), zero))
			{
				// In triangle!
				result = true;
			}
		}
	}

	return result;
}

PRIVATE void JWSystemPhysics::PickEntityBySphere() noexcept
{
	m_PickedEntityName.clear();

	if (m_vpComponents.size())
	{
		for (auto iter : m_vpComponents)
		{
			auto entity = iter->PtrEntity;

			auto transform{ entity->GetComponentTransform() };
			auto type{ entity->GetEntityType() };

			if (type != EEntityType::UserDefined)
			{
				// MainSprite needs to be picked
				if (type != EEntityType::MainSprite)
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
					// You must not be able to pick the current camera!
					if (m_pECS->SystemCamera().GetCurrentCameraComponentID() == camera->ComponentID)
					{
						continue;
					}
				}

				auto& radius = iter->BoundingSphereRadius;
				auto center = iter->BoundingSphereCenterPosition;

				// Sphere equation
				// (Px - Cx)©÷ + (Py - Cy)©÷ + (Pz - Cz)©÷ = r©÷
				// Dot(P - C, P - C) = r©÷
				// # P = any point on the sphere  # C = center of the sphere  # r = radius of the sphere
				//
				// Line's parametric equation
				// L = O + tD
				// (L for Line, O for Ray Origin, D for Ray Direction)
				//
				// Line-Sphere intersection (which is the simultaneous equation of the two equations)
				// Dot(L - C, L - C) = r©÷
				// Dot(O + tD - C, O + tD - C) = r©÷
				// (Ox + t*Dx - Cx)©÷ + (Oy + t*Dy - Cy)©÷ + (Oz + t*Dz - Cz)©÷ = r©÷
				// (Ox + t*Dx - Cx)©÷ + (Oy + t*Dy - Cy)©÷ + (Oz + t*Dz - Cz)©÷ - r©÷ = 0
				//
				// Now, let's expand the equation
				// Ox©÷ + t©÷Dx©÷ + Cx©÷ + 2tOxDx -2OxCx -2tDxCx + Oy©÷ + t©÷Dy©÷ + Cy©÷ + 2tOyDy -2OyCy -2tDyCy
				//  + Oz©÷ + t©÷Dz©÷ + Cz©÷ + 2tOzDz -2OzCz -2tDzCz - r©÷ = 0
				//
				// Let's simplify it using Dot()
				// Dot(O, O) + t©÷Dot(D, D) + Dot(C, C) + 2tDot(O, D) - 2Dot(O, C) - 2tDot(D, C) - r©÷ = 0
				//
				// Let's rearrange it by 't'
				// t©÷Dot(D, D) + 2t(Dot(D, O) - Dot(D, C)) + Dot(O, O) - 2Dot(O, C) + Dot(C, C) - r©÷ = 0
				//
				// Optimize Dot() #0
				// t©÷Dot(D, D) + 2t(Dot(D, O - C)) + Dot(O, O) - 2Dot(O, C) + Dot(C, C) - r©÷ = 0
				//
				// Optimize Dot() #1
				// t©÷Dot(D, D) + 2t(Dot(D, O - C)) + Dot(O - C, O - C) - r©÷ = 0
				//
				// That wil give us the following quadratic equation
				// at©÷ + bt + c = 0
				// # a = Dot(D, D)  # b = 2 * Dot(D, O - C)  # c = Dot(O - C, O - C) - r©÷
				//
				// discriminant of quadratic equation: b©÷ - 4ac
				//
				// And if, b©÷ - 4ac ¡Ã 0
				// then, the ray hit the sphere!
				
				auto r = XMVectorSet(radius, radius, radius, 1.0f);
				auto a = XMVector3Dot(m_PickingRayDirection, m_PickingRayDirection);
				auto b = 2.0f * XMVector3Dot(m_PickingRayDirection, m_PickingRayOrigin - center);
				auto c = XMVector3Dot(m_PickingRayOrigin - center, m_PickingRayOrigin - center) - r * r;
				auto discriminant = b * b - 4.0f * a * c;

				XMVECTOR t{};
				if (XMVector3GreaterOrEqual(discriminant, XMVectorZero()))
				{
					m_pPickedEntity = entity;
					m_PickedEntityName = entity->GetEntityName();
				}
			}
		}
	}
}

auto JWSystemPhysics::GetPickingRayOrigin() const noexcept->const XMVECTOR& 
{ 
	return m_PickingRayOrigin; 
}

auto JWSystemPhysics::GetPickingRayDirection() const noexcept->const XMVECTOR&
{ 
	return m_PickingRayDirection;
}

auto JWSystemPhysics::GetPickedEntityName() const noexcept->const STRING&
{
	return m_PickedEntityName;
}

auto JWSystemPhysics::GetPickedTrianglePosition(uint32_t PositionIndex) const noexcept->const XMVECTOR&
{
	PositionIndex = min(PositionIndex, 2);

	return m_PickedTriangle[PositionIndex];
}

void JWSystemPhysics::Execute() noexcept
{
	for (auto& iter : m_vpComponents)
	{
		auto transform{ iter->PtrEntity->GetComponentTransform() };
		if (transform)
		{
			if (transform->ShouldUpdateBoundingVolume)
			{
				UpdateBoundingSphere(iter->PtrEntity);

				transform->ShouldUpdateBoundingVolume = false;
			}
		}
	}
}
#include "JWECS.h"
#include "../Core/JWMath.h"

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

void JWSystemPhysics::SetBoundingEllipsoid(JWEntity* pEntity, const SBoundingEllipsoidData& Data) noexcept
{
	if (pEntity == nullptr) { return; }
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Set the radius (RadiusX,Y,Z)
	physics->BoundingEllipsoid.RadiusX = Data.RadiusX;
	physics->BoundingEllipsoid.RadiusY = Data.RadiusY;
	physics->BoundingEllipsoid.RadiusZ = Data.RadiusZ;

	// Set the offset
	physics->BoundingEllipsoid.Offset = Data.Offset;

	UpdateBoundingEllipsoid(pEntity);
}

void JWSystemPhysics::HideBoundingEllipsoid(JWEntity* pEntity) noexcept
{
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	auto& bounding_ellipsoid = physics->BoundingEllipsoid;
	bounding_ellipsoid.EllipsoidWorld = XMMatrixScaling(0, 0, 0);

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingEllipsoidInstance(physics->ComponentID, bounding_ellipsoid.EllipsoidWorld);
}

void JWSystemPhysics::UpdateBoundingEllipsoid(JWEntity* pEntity) noexcept
{
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Calculate world matrix of the bounding ellipsoid
	auto transform = pEntity->GetComponentTransform();
	auto& bounding_ellipsoid = physics->BoundingEllipsoid;

	auto temp_center = bounding_ellipsoid.Offset;
	if (transform) { temp_center += transform->Position; }

	auto mat_scaling = XMMatrixScaling(bounding_ellipsoid.RadiusX, bounding_ellipsoid.RadiusY, bounding_ellipsoid.RadiusZ);
	auto mat_translation = XMMatrixTranslationFromVector(temp_center);
	bounding_ellipsoid.EllipsoidWorld = mat_scaling * mat_translation;

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingEllipsoidInstance(physics->ComponentID, bounding_ellipsoid.EllipsoidWorld);
}

void JWSystemPhysics::SetSubBoundingEllipsoids(JWEntity* pEntity, const VECTOR<SBoundingEllipsoidData>& vData) noexcept
{
	if (pEntity == nullptr) { return; }
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Set sub-bounding-ellipsoids
	physics->SubBoundingEllipsoids = vData;

	// Calculate world matrix of the bounding ellipsoid
	auto transform = pEntity->GetComponentTransform();
	auto entity_position = transform->Position;

	for (auto& iter : physics->SubBoundingEllipsoids)
	{
		auto mat_scaling = XMMatrixScaling(iter.RadiusX, iter.RadiusY, iter.RadiusZ);
		auto mat_translation = XMMatrixTranslationFromVector(entity_position + iter.Offset);
		iter.EllipsoidWorld = mat_scaling * mat_translation;
	}
}

auto JWSystemPhysics::PickEntity() noexcept->bool
{
	m_pPickedEntity = nullptr;
	m_IsEntityPicked = false;
	m_PickedEntityName.clear();

	CastPickingRay();

	if (m_vpComponents.size() == 0)
	{
		return false;
	}

	if (PickEntityByEllipsoid())
	{
		m_IsEntityPicked = true;
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
	m_NormalizedMousePosition.x = (static_cast<float>(m_MouseClientPosition.x) / m_WindowSize.x) * 2.0f - 1.0f;
	m_NormalizedMousePosition.y = (static_cast<float>(m_MouseClientPosition.y) / m_WindowSize.y) * 2.0f - 1.0f;

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

PRIVATE auto JWSystemPhysics::PickEntityByTriangle() noexcept->bool
{
	XMVECTOR old_t{ XMVectorSet(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX) };

	for (auto iter : m_vpComponents)
	{
		auto& entity = iter->PtrEntity;

		auto transform{ entity->GetComponentTransform() };
		auto render{ entity->GetComponentRender() };
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
			auto model_type = render->PtrModel->GetRenderType();

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

			const auto& faces{ ptr_index_data->vFaces };
			const auto& vertices{ ptr_vertex_data->vVerticesModel };

			// Iterate all the triangles in the model
			for (auto triangle : faces)
			{
				// Move vertices from local space to world space!
				auto v0 = XMVector3TransformCoord(vertices[triangle._0].Position, transform->WorldMatrix);
				auto v1 = XMVector3TransformCoord(vertices[triangle._1].Position, transform->WorldMatrix);
				auto v2 = XMVector3TransformCoord(vertices[triangle._2].Position, transform->WorldMatrix);

				if (IntersectRayTriangle(m_PickedPoint, old_t,
					m_PickingRayOrigin, m_PickingRayDirection,
					v0, v1, v2))
				{
					m_pPickedEntity = entity;

					m_PickedTriangle[0] = v0;
					m_PickedTriangle[1] = v1;
					m_PickedTriangle[2] = v2;
				}
			}
		}
	}

	if (m_pPickedEntity)
	{
		m_PickedEntityName = m_pPickedEntity->GetEntityName();
		return true;
	}

	return false;
}

PRIVATE auto JWSystemPhysics::PickEntityByEllipsoid() noexcept->bool
{
	XMVECTOR old_t{ D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX };
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

			if (IntersectRayUnitSphere(e_ray_origin, e_ray_direction, &old_t))
			{
				m_PickedPoint = m_PickingRayOrigin + old_t * m_PickingRayDirection;
				m_pPickedEntity = entity;
			}
		}
	}

	if (m_pPickedEntity)
	{
		m_PickedEntityName = m_pPickedEntity->GetEntityName();
		m_PickedPoint = m_PickingRayOrigin + old_t * m_PickingRayDirection;
		return true;
	}

	return false;
}

auto JWSystemPhysics::PickSubBoundingEllipsoid() noexcept->bool
{
	if (m_pPickedEntity == nullptr) { return false; }
	auto physics = m_pPickedEntity->GetComponentPhysics();
	if (physics == nullptr) { return false; }
	auto transform = m_pPickedEntity->GetComponentTransform();
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

		// @important: NO old_t comparison!
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

void JWSystemPhysics::PickTerrainTriangle() noexcept
{
	if (m_pPickedEntity)
	{
		auto render = m_pPickedEntity->GetComponentRender();
		if (render == nullptr) { return; }

		auto ptr_terrain = render->PtrTerrain;
		if (ptr_terrain == nullptr) { return; }

		auto transform = m_pPickedEntity->GetComponentTransform();
		
		XMVECTOR v[3]{};
		XMVECTOR old_t{ XMVectorSet(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX) };
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

						if (IntersectRayTriangle(m_PickedPoint, old_t,
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

		m_PickedPoint = m_PickingRayOrigin + old_t * m_PickingRayDirection;
	}
}

void JWSystemPhysics::Execute() noexcept
{
	for (auto& iter : m_vpComponents)
	{
		auto transform{ iter->PtrEntity->GetComponentTransform() };
		if (transform)
		{
			if (transform->ShouldUpdateBoundingEllipsoid)
			{
				UpdateBoundingEllipsoid(iter->PtrEntity);

				transform->ShouldUpdateBoundingEllipsoid = false;
			}
		}
	}
}
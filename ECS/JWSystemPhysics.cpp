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

	// Set center position
	auto transform = m_vpComponents[slot]->PtrEntity->GetComponentTransform();
	if (transform)
	{
		m_vpComponents[slot]->BoundingSphereData.Center =
			m_vpComponents[slot]->BoundingSphereData.Offset + transform->Position;
	}

	// Add bounding volume instance into SystemRender
	m_pECS->SystemRender().AddBoundingVolumeInstance(m_vpComponents[slot]->BoundingSphereData.Radius, m_vpComponents[slot]->BoundingSphereData.Center);

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
		m_vpComponents[slot]->ComponentID = slot; // @important

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
	physics->BoundingSphereData.Radius = Radius;

	// Set center position
	auto transform = physics->PtrEntity->GetComponentTransform();
	physics->BoundingSphereData.Offset = XMVectorSet(CenterOffset.x, CenterOffset.y, CenterOffset.z, 1);
	physics->BoundingSphereData.Center = physics->BoundingSphereData.Offset + transform->Position;
	
	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingVolumeInstance(physics->ComponentID, physics->BoundingSphereData.Radius, physics->BoundingSphereData.Center);
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
	m_pECS->SystemRender().UpdateBoundingVolumeInstance(physics->ComponentID, physics->BoundingSphereData.Radius, physics->BoundingSphereData.Center);
}

void JWSystemPhysics::UpdateBoundingSphere(JWEntity* pEntity) noexcept
{
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Set center position
	auto transform = physics->PtrEntity->GetComponentTransform();
	if (transform)
	{
		physics->BoundingSphereData.Center = physics->BoundingSphereData.Offset + transform->Position;
	}

	// Update the data into SystemRender
	m_pECS->SystemRender().UpdateBoundingVolumeInstance(physics->ComponentID, physics->BoundingSphereData.Radius, physics->BoundingSphereData.Center);
}

void JWSystemPhysics::SetSubBoundingSpheres(JWEntity* pEntity, const VECTOR<SBoundingSphereData>& vData) noexcept
{
	if (pEntity == nullptr) { return; }
	auto physics = pEntity->GetComponentPhysics();
	if (physics == nullptr) { return; }

	// Set sub-bounding-spheres
	physics->SubBoundingSpheres = vData;
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

	if (PickEntityBySphere())
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

PRIVATE auto JWSystemPhysics::PickEntityBySphere() noexcept->bool
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

			const auto& center = iter->BoundingSphereData.Center;
			const auto& radius = iter->BoundingSphereData.Radius;
				
			if (IntersectRaySphere(m_PickingRayOrigin, m_PickingRayDirection, center, radius, &old_t))
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

auto JWSystemPhysics::PickSubBoundingSphere() noexcept->bool
{
	if (m_pPickedEntity == nullptr) { return false; }
	auto physics = m_pPickedEntity->GetComponentPhysics();
	if (physics == nullptr) { return false; }

	// Clear
	m_vPickedSubBSID.clear();
	
	auto& sub_bs = physics->SubBoundingSpheres;
	XMVECTOR old_t{ D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX };
	for (uint32_t id = static_cast<uint32_t>(sub_bs.size()); id > 0; --id)
	{
		if (IntersectRaySphere(m_PickingRayOrigin, m_PickingRayDirection, sub_bs[id - 1].Center, sub_bs[id - 1].Radius))
		{
			m_vPickedSubBSID.push_back(id - 1);
		}
	}

	if (m_vPickedSubBSID.size())
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
		
		XMVECTOR old_t{ XMVectorSet(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX) };
		for (auto& bs : m_vPickedSubBSID)
		{
			for (auto& node : ptr_terrain->QuadTree)
			{
				if (node.SubBoundingSphereID == bs)
				{
					const auto& faces{ node.IndexData.vFaces };
					const auto& vertices{ node.VertexData.vVerticesModel };
					for (auto triangle : faces)
					{
						if (IntersectRayTriangle(m_PickedPoint, old_t,
							m_PickingRayOrigin, m_PickingRayDirection,
							vertices[triangle._0].Position, vertices[triangle._1].Position, vertices[triangle._2].Position))
						{
							m_PickedTriangle[0] = vertices[triangle._0].Position;
							m_PickedTriangle[1] = vertices[triangle._1].Position;
							m_PickedTriangle[2] = vertices[triangle._2].Position;
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
			if (transform->ShouldUpdateBoundingVolume)
			{
				UpdateBoundingSphere(iter->PtrEntity);

				transform->ShouldUpdateBoundingVolume = false;
			}
		}
	}
}
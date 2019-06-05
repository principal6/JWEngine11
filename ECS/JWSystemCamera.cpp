#include "JWECS.h"
#include "../Core/JWDX.h"

using namespace JWEngine;

void JWSystemCamera::Create(JWECS& ECS, JWDX& DX, const SSize2& WindowSize) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;

	// Set JWDX pointer.
	m_pDX = &DX;

	m_pWindowSize = &WindowSize;
}

void JWSystemCamera::Destroy() noexcept {}

PRIVATE auto JWSystemCamera::CreateComponent(EntityIndexType EntityIndex) noexcept->ComponentIndexType
{
	auto component_index{ static_cast<ComponentIndexType>(m_vComponents.size()) };

	// @important
	// Save component ID & pointer to Entity & pointer to WindowSize
	m_vComponents.emplace_back(EntityIndex, component_index, m_pWindowSize);

	return component_index;
}

PRIVATE void JWSystemCamera::DestroyComponent(ComponentIndexType ComponentIndex) noexcept
{
	if (m_vComponents.size() == 0)
	{
		JW_ERROR_ABORT("There is no component to destroy.");
	}

	if (m_vComponents.size() == 1)
	{
		// 아래 코드 실행하면... 프로그램 종료될 때도 해제 실패한다고 에러 띄움!!
		//JW_ERROR_RETURN("카메라 컴포넌트 해제 실패. 모든 장면에는 최소한 1개의 카메라가 필요합니다.");
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

	// Swap the last element of the vector and the deleted element if necessary
	if (component_index < last_index)
	{
		m_vComponents[component_index] = std::move(m_vComponents[last_index]);
		m_vComponents[component_index].ComponentIndex = component_index; // @important

		ptr_entity_last_component->SetComponentCameraIndex(component_index); // @important
	}

	// Shrink the size of the vector.
	m_vComponents.pop_back();
}

PRIVATE auto JWSystemCamera::GetComponentPtr(ComponentIndexType ComponentIndex) noexcept->SComponentCamera*
{
	if (ComponentIndex >= m_vComponents.size())
	{
		return nullptr;
	}

	return &m_vComponents[ComponentIndex];
}

void JWSystemCamera::CaptureViewFrustum() noexcept
{
	const auto& view_matrix = m_pCurrentCamera->MatrixView;
	auto inverse_view_matrix = XMMatrixInverse(nullptr, view_matrix);

	auto& nru = m_CapturedViewFrustumVertices.NRU;
	auto& nld = m_CapturedViewFrustumVertices.NLD;
	auto& nlu = m_CapturedViewFrustumVertices.NLU;
	auto& nrd = m_CapturedViewFrustumVertices.NRD;

	auto& fru = m_CapturedViewFrustumVertices.FRU;
	auto& fld = m_CapturedViewFrustumVertices.FLD;
	auto& flu = m_CapturedViewFrustumVertices.FLU;
	auto& frd = m_CapturedViewFrustumVertices.FRD;
	
	nru = GetCurrentCameraViewFrustumNRU();
	nld = XMVectorSetZ(-nru, XMVectorGetZ(nru));
	nlu = XMVectorSetY(nld, XMVectorGetY(nru));
	nrd = XMVectorSetY(nru, XMVectorGetY(nld));

	fru = GetCurrentCameraViewFrustumFRU();
	fld = XMVectorSetZ(-fru, XMVectorGetZ(fru));
	flu = XMVectorSetY(fld, XMVectorGetY(fru));
	frd = XMVectorSetY(fru, XMVectorGetY(fld));

	// Move vertices to World Space.
	nru = XMVector3TransformCoord(nru, inverse_view_matrix);
	nld = XMVector3TransformCoord(nld, inverse_view_matrix);
	nlu = XMVector3TransformCoord(nlu, inverse_view_matrix);
	nrd = XMVector3TransformCoord(nrd, inverse_view_matrix);

	fru = XMVector3TransformCoord(fru, inverse_view_matrix);
	fld = XMVector3TransformCoord(fld, inverse_view_matrix);
	flu = XMVector3TransformCoord(flu, inverse_view_matrix);
	frd = XMVector3TransformCoord(frd, inverse_view_matrix);

	return;
}

PRIVATE inline auto JWSystemCamera::GetCurrentCameraViewFrustumNRU() const noexcept->XMVECTOR
{
	XMFLOAT3 position{};
	position.z = m_pCurrentCamera->ZNear;
	position.y = tanf(m_pCurrentCamera->FOV / 2.0f) * position.z;
	position.x = position.y * m_pWindowSize->floatX() / m_pWindowSize->floatY();

	return XMVectorSet(position.x, position.y, position.z, 1);
}

PRIVATE inline auto JWSystemCamera::GetCurrentCameraViewFrustumFRU() const noexcept->XMVECTOR
{
	XMFLOAT3 position{};
	position.z = m_pCurrentCamera->ZFar * 0.98f;
	position.y = tanf(m_pCurrentCamera->FOV / 2.0f) * position.z;
	position.x = position.y * m_pWindowSize->floatX() / m_pWindowSize->floatY();

	return XMVectorSet(position.x, position.y, position.z, 1);
}

void JWSystemCamera::Execute() noexcept
{
	if (m_pCurrentCamera == nullptr)
	{
		if (m_vComponents.size())
		{
			SetCurrentCamera(0);
		}
		else
		{
			JW_ERROR_ABORT("카메라가 없습니다.");
		}
	}

	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	// Update current camera's position to PS for specular calculation.
	auto transform = ptr_entity->GetComponentTransform();
	m_pDX->UpdatePSCBCamera(transform->Position);
}

void JWSystemCamera::SetCurrentCamera(size_t ComponentID) noexcept
{
	if (m_vComponents.size() == 0)
	{
		JW_ERROR_ABORT("You didn't create any camera.");
	}

	ComponentID = min(ComponentID, m_vComponents.size() - 1);

	m_pCurrentCamera = &m_vComponents[ComponentID];

	RotateCurrentCamera(0, 0, 0);
	UpdateCurrentCameraViewMatrix();
}

void JWSystemCamera::UpdateCamerasProjectionMatrix() noexcept
{
	if (m_vComponents.size())
	{
		for (auto& iter : m_vComponents)
		{
			if (iter.Type == ECameraType::Orthographic)
			{
				iter.MatrixProjection =
					XMMatrixOrthographicLH(m_pWindowSize->floatX(), m_pWindowSize->floatY(), iter.ZNear, iter.ZFar);
			}
			else
			{
				iter.MatrixProjection =
					XMMatrixPerspectiveFovLH(iter.FOV, m_pWindowSize->floatX() / m_pWindowSize->floatY(), iter.ZNear, iter.ZFar);
			}
		}
	}
}

void JWSystemCamera::RotateCurrentCamera(float X_Pitch, float Y_Yaw, float Z_Roll) noexcept
{
	const auto& type = m_pCurrentCamera->Type;
	const auto& rotate_factor = m_pCurrentCamera->RotateFactor;

	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	auto transform = ptr_entity->GetComponentTransform();
	transform->RotatePitchYawRoll(XMFLOAT3(X_Pitch * rotate_factor, Y_Yaw * rotate_factor, Z_Roll * rotate_factor), true);
	auto& position = transform->Position;
	const auto& forward = transform->Forward;

	// Update Position/LookAt vector.
	auto& lookat = m_pCurrentCamera->LookAt;
	if (type == ECameraType::ThirdPerson)
	{
		const auto& zoom = m_pCurrentCamera->Zoom;
		position = lookat - forward * zoom;
	}
	else
	{
		lookat = position + forward;
	}

	UpdateCurrentCameraViewMatrix();
}

void JWSystemCamera::ZoomCurrentCamera(float Factor) noexcept
{
	const auto& type = m_pCurrentCamera->Type;
	if (type != ECameraType::ThirdPerson)
	{
		return;
	}

	const auto& zoom_factor = m_pCurrentCamera->ZoomFactor;
	const auto& zoom_near = m_pCurrentCamera->ZoomNear;
	const auto& zoom_far = m_pCurrentCamera->ZoomFar;

	// Update Zoom value.
	auto& zoom = m_pCurrentCamera->Zoom;
	zoom += -Factor * zoom_factor;
	zoom = max(zoom, zoom_near);
	zoom = min(zoom, zoom_far);
	
	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	// Update Position vector.
	auto transform = ptr_entity->GetComponentTransform();
	auto& position = transform->Position;
	const auto& forward = transform->Forward;
	const auto& lookat = m_pCurrentCamera->LookAt;
	position = lookat - forward * zoom;

	UpdateCurrentCameraViewMatrix();
}

void JWSystemCamera::MoveCurrentCamera(ECameraDirection Direction) noexcept
{
	const auto& type = m_pCurrentCamera->Type;
	switch (type)
	{
	case JWEngine::ECameraType::FreeLook:
		MoveFreeLook(Direction);
		break;
	case JWEngine::ECameraType::FirstPerson:
		MoveFirstPerson(Direction);
		break;
	case JWEngine::ECameraType::ThirdPerson:
		MoveThirdPerson(Direction);
		break;
	default:
		break;
	}

	UpdateCurrentCameraViewMatrix();
}

PRIVATE inline void JWSystemCamera::MoveFreeLook(ECameraDirection Direction) noexcept
{
	const auto& move_factor = m_pCurrentCamera->MoveFactor;

	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	auto transform = ptr_entity->GetComponentTransform();
	auto& position = transform->Position;
	const auto& forward = transform->Forward;
	const auto& right = transform->Right;

	// Update Position vector.
	switch (Direction)
	{
	case JWEngine::ECameraDirection::Forward:
		position += forward * move_factor;
		break;
	case JWEngine::ECameraDirection::Backward:
		position -= forward * move_factor;
		break;
	case JWEngine::ECameraDirection::Left:
		position -= right * move_factor;
		break;
	case JWEngine::ECameraDirection::Right:
		position += right * move_factor;
		break;
	default:
		break;
	}

	// Update LookAt vector.
	auto& lookat = m_pCurrentCamera->LookAt;
	lookat = position + forward;
}

PRIVATE inline void JWSystemCamera::MoveFirstPerson(ECameraDirection Direction) noexcept
{
	const auto& move_factor = m_pCurrentCamera->MoveFactor;

	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	auto transform = ptr_entity->GetComponentTransform();
	auto& position = transform->Position;
	const auto& forward = transform->Forward;
	const auto& right = transform->Right;

	auto temp_forward = XMVectorSetY(forward, 0);
	auto temp_right = XMVectorSetY(right, 0);

	// Update Position vector.
	switch (Direction)
	{
	case JWEngine::ECameraDirection::Forward:
		position += temp_forward * move_factor;
		break;
	case JWEngine::ECameraDirection::Backward:
		position -= temp_forward * move_factor;
		break;
	case JWEngine::ECameraDirection::Left:
		position -= temp_right * move_factor;
		break;
	case JWEngine::ECameraDirection::Right:
		position += temp_right * move_factor;
		break;
	default:
		break;
	}

	// Update LookAt vector.
	auto& lookat = m_pCurrentCamera->LookAt;
	lookat = position + forward;
}

PRIVATE inline void JWSystemCamera::MoveThirdPerson(ECameraDirection Direction) noexcept
{
	const auto& move_factor = m_pCurrentCamera->MoveFactor;

	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	auto transform = ptr_entity->GetComponentTransform();
	const auto& forward = transform->Forward;
	const auto& right = transform->Right;

	auto temp_forward = XMVectorSetY(forward, 0);
	auto temp_right = XMVectorSetY(right, 0);

	// Update LookAt vector.
	auto& lookat = m_pCurrentCamera->LookAt;
	switch (Direction)
	{
	case JWEngine::ECameraDirection::Forward:
		lookat += temp_forward * move_factor;
		break;
	case JWEngine::ECameraDirection::Backward:
		lookat -= temp_forward * move_factor;
		break;
	case JWEngine::ECameraDirection::Left:
		lookat -= temp_right * move_factor;
		break;
	case JWEngine::ECameraDirection::Right:
		lookat += temp_right * move_factor;
		break;
	default:
		break;
	}

	// Update Position vector.
	auto& position = transform->Position;
	const auto& zoom = m_pCurrentCamera->Zoom;
	position = lookat - forward * zoom;
}

PRIVATE inline void JWSystemCamera::UpdateCurrentCameraViewMatrix() noexcept
{
	auto& matrix_view = m_pCurrentCamera->MatrixView;
	
	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	const auto transform = ptr_entity->GetComponentTransform();
	const auto& position = transform->Position;
	const auto& up = transform->Up;

	const auto& lookat = m_pCurrentCamera->LookAt;
	const auto& zoom = m_pCurrentCamera->Zoom;
	
	matrix_view = XMMatrixLookAtLH(position, lookat, up);
}

void JWSystemCamera::SetCurrentCameraPosition(const XMFLOAT3& Position) noexcept
{
	const auto& type = m_pCurrentCamera->Type;

	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	auto transform = ptr_entity->GetComponentTransform();
	auto& position = transform->Position = XMVectorSet(Position.x, Position.y, Position.z, 1);
	auto& lookat = m_pCurrentCamera->LookAt;
	const auto& forward = transform->Forward;

	if (type == ECameraType::ThirdPerson)
	{
		lookat = XMVectorSet(Position.x, Position.y, Position.z, 0);

		const auto& zoom = m_pCurrentCamera->Zoom;
		position = lookat - forward * zoom;
	}
	else
	{
		position = XMVectorSet(Position.x, Position.y, Position.z, 0);

		lookat = position + forward;
	}

	UpdateCurrentCameraViewMatrix();
}

auto JWSystemCamera::GetCurrentCameraPosition() const noexcept->const XMVECTOR&
{
	// Get pointer to the entity.
	auto ptr_entity = m_pECS->GetEntityByIndex(m_pCurrentCamera->EntityIndex);

	return ptr_entity->GetComponentTransform()->Position;
}
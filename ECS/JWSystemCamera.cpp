#include "JWECS.h"
#include "../Core/JWDX.h"

using namespace JWEngine;

void JWSystemCamera::Create(JWECS& ECS, JWDX& DX, XMFLOAT2 WindowSize) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;

	// Set JWDX pointer.
	m_pDX = &DX;
	
	// Set orthographic projection matrix.
	m_MatrixProjOrthographic = XMMatrixOrthographicLH(WindowSize.x, WindowSize.y, KOrthographicNearZ, KOrthographicFarZ);
}

void JWSystemCamera::Destroy() noexcept
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}
}

auto JWSystemCamera::CreateComponent(JWEntity* pEntity) noexcept->SComponentCamera&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };

	auto new_entry{ new SComponentCamera() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID & pointer to Entity
	m_vpComponents[slot]->ComponentID = slot;
	m_vpComponents[slot]->PtrEntity = pEntity;

	return *m_vpComponents[slot];
}

void JWSystemCamera::DestroyComponent(SComponentCamera& Component) noexcept
{
	if (m_vpComponents.size() == 1)
	{
		// 아래처럼 하면... 프로그램 종료될 때도 해제 실패함!!
		//JW_ERROR_RETURN("카메라 컴포넌트 해제 실패. 모든 장면에는 최소한 1개의 카메라가 필요합니다.");
	}

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
	position.x = position.y * (m_pCurrentCamera->Width) / (m_pCurrentCamera->Height);

	return XMVectorSet(position.x, position.y, position.z, 1);
}

PRIVATE inline auto JWSystemCamera::GetCurrentCameraViewFrustumFRU() const noexcept->XMVECTOR
{
	XMFLOAT3 position{};
	position.z = m_pCurrentCamera->ZFar * 0.98f;
	position.y = tanf(m_pCurrentCamera->FOV / 2.0f) * position.z;
	position.x = position.y * (m_pCurrentCamera->Width) / (m_pCurrentCamera->Height);

	return XMVectorSet(position.x, position.y, position.z, 1);
}

void JWSystemCamera::Execute() noexcept
{
	if (m_pCurrentCamera == nullptr)
	{
		if (m_vpComponents.size())
		{
			SetCurrentCamera(0);
		}
		else
		{
			JW_ERROR_ABORT("카메라가 없습니다.");
		}
	}

	// Update current camera's position to PS for specular calculation.
	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	m_pDX->UpdatePSCBCamera(transform->Position);
}

void JWSystemCamera::SetCurrentCamera(size_t ComponentID) noexcept
{
	if (m_vpComponents.size() == 0)
	{
		JW_ERROR_ABORT("You didn't create any camera.");
	}

	ComponentID = min(ComponentID, m_vpComponents.size() - 1);

	if (m_pCurrentCamera)
	{
		m_pECS->SystemPhysics().UnhideBoundingSphere(m_pCurrentCamera->PtrEntity);
	}

	m_pCurrentCamera = m_vpComponents[ComponentID];

	m_pECS->SystemPhysics().HideBoundingSphere(m_pCurrentCamera->PtrEntity);

	RotateCurrentCamera(0, 0, 0);
	UpdateCurrentCameraViewMatrix();
}

void JWSystemCamera::RotateCurrentCamera(float X_Pitch, float Y_Yaw, float Z_Roll) noexcept
{
	const auto& type = m_pCurrentCamera->Type;
	const auto& rotate_factor = m_pCurrentCamera->RotateFactor;

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	transform->RotatePitchYawRoll(X_Pitch * rotate_factor, Y_Yaw * rotate_factor, Z_Roll * rotate_factor, true);
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
	
	// Update Position vector.
	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
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

	// Update Camera's bounding sphere position & hide it.
	m_pECS->SystemPhysics().UpdateBoundingSphere(m_pCurrentCamera->PtrEntity);
	m_pECS->SystemPhysics().HideBoundingSphere(m_pCurrentCamera->PtrEntity);

	UpdateCurrentCameraViewMatrix();
}

PRIVATE inline void JWSystemCamera::MoveFreeLook(ECameraDirection Direction) noexcept
{
	const auto& move_factor = m_pCurrentCamera->MoveFactor;

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
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

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
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

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
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
	
	const auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	const auto& position = transform->Position;
	const auto& up = transform->Up;

	const auto& lookat = m_pCurrentCamera->LookAt;
	const auto& zoom = m_pCurrentCamera->Zoom;
	
	matrix_view = XMMatrixLookAtLH(position, lookat, up);
}

void JWSystemCamera::SetCurrentCameraPosition(const XMFLOAT3& Position) noexcept
{
	const auto& type = m_pCurrentCamera->Type;

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
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
	return m_pCurrentCamera->PtrEntity->GetComponentTransform()->Position;
}
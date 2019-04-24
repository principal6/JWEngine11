#include "JWECS.h"

using namespace JWEngine;

void JWSystemCamera::Create(JWECS& ECS, XMFLOAT2 WindowSize) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;
	
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

auto JWSystemCamera::CreateComponent() noexcept->SComponentCamera&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };

	auto new_entry{ new SComponentCamera() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID
	m_vpComponents[slot]->ComponentID = slot;

	return *m_vpComponents[slot];
}

void JWSystemCamera::DestroyComponent(SComponentCamera& Component) noexcept
{
	if (m_vpComponents.size() == 1)
	{
		JW_ERROR_RETURN("카메라 컴포넌트 해제 실패. 모든 장면에는 최소한 1개의 카메라가 필요합니다.");
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

void JWSystemCamera::SetMainCamera(size_t CameraIndex) noexcept
{
	if (m_vpComponents.size() == 0)
	{
		JW_ERROR_ABORT("You didn't create any camera.");
	}

	CameraIndex = max(CameraIndex, m_vpComponents.size() - 1);

	m_pCurrentCamera = m_vpComponents[CameraIndex];
}

void JWSystemCamera::RotateCurrentCamera(float X_Pitch, float Y_Yaw, float Z_Roll) noexcept
{
	const auto& type = m_pCurrentCamera->Type;
	const auto& rotate_factor = m_pCurrentCamera->RotateFactor;

	auto& pitch = m_pCurrentCamera->Pitch;
	auto& yaw = m_pCurrentCamera->Yaw;
	auto& roll = m_pCurrentCamera->Roll;

	pitch += X_Pitch * rotate_factor;
	yaw += Y_Yaw * rotate_factor;
	roll += Z_Roll * rotate_factor;

	// This is needed to eliminate Y-axis flipping
	pitch = min(pitch, XM_PIDIV2 - 0.01f);
	pitch = max(pitch, -XM_PIDIV2 + 0.01f);

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	auto& position = transform->Position;
	auto& orientation = transform->Orientation;

	// Update Orientation vector.
	const auto& default_forward = m_pCurrentCamera->DefaultForward;
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	orientation = XMVector3TransformNormal(default_forward, rotation);

	// Update Position/LookAt vector.
	auto& lookat = m_pCurrentCamera->LookAt;
	if (type == ECameraType::ThirdPerson)
	{
		const auto& zoom = m_pCurrentCamera->Zoom;
		position = lookat - orientation * zoom;
	}
	else
	{
		lookat = position + orientation;
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
	const auto& orientation = transform->Orientation;
	const auto& lookat = m_pCurrentCamera->LookAt;
	position = lookat - orientation * zoom;

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

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	auto& position = transform->Position;
	auto& orientation = transform->Orientation;

	const auto& up = m_pCurrentCamera->Up;
	const auto right = XMVector3Normalize(XMVector3Cross(up, orientation));

	// Update Position vector.
	switch (Direction)
	{
	case JWEngine::ECameraDirection::Forward:
		position += orientation * move_factor;
		break;
	case JWEngine::ECameraDirection::Backward:
		position -= orientation * move_factor;
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
	lookat = position + orientation;
}

PRIVATE inline void JWSystemCamera::MoveFirstPerson(ECameraDirection Direction) noexcept
{
	const auto& move_factor = m_pCurrentCamera->MoveFactor;

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	auto& position = transform->Position;
	const auto& orientation = transform->Orientation;

	const auto& up = m_pCurrentCamera->Up;
	const auto right = XMVector3Normalize(XMVector3Cross(up, orientation));

	auto temp_orientation = XMVectorSetY(orientation, 0);
	auto temp_right = XMVectorSetY(right, 0);

	// Update Position vector.
	switch (Direction)
	{
	case JWEngine::ECameraDirection::Forward:
		position += temp_orientation * move_factor;
		break;
	case JWEngine::ECameraDirection::Backward:
		position -= temp_orientation * move_factor;
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
	lookat = position + orientation;
}

PRIVATE inline void JWSystemCamera::MoveThirdPerson(ECameraDirection Direction) noexcept
{
	const auto& move_factor = m_pCurrentCamera->MoveFactor;

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	const auto& orientation = transform->Orientation;

	const auto& up = m_pCurrentCamera->Up;
	const auto right = XMVector3Normalize(XMVector3Cross(up, orientation));

	auto temp_orientation = XMVectorSetY(orientation, 0);
	auto temp_right = XMVectorSetY(right, 0);

	// Update LookAt vector.
	auto& lookat = m_pCurrentCamera->LookAt;
	switch (Direction)
	{
	case JWEngine::ECameraDirection::Forward:
		lookat += temp_orientation * move_factor;
		break;
	case JWEngine::ECameraDirection::Backward:
		lookat -= temp_orientation * move_factor;
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
	position = lookat - orientation * zoom;
}

PRIVATE inline void JWSystemCamera::UpdateCurrentCameraViewMatrix() noexcept
{
	auto& matrix_view = m_pCurrentCamera->MatrixView;
	
	const auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	const auto& position = transform->Position;
	
	const auto& lookat = m_pCurrentCamera->LookAt;
	const auto& zoom = m_pCurrentCamera->Zoom;
	const auto& up = m_pCurrentCamera->Up;

	matrix_view = XMMatrixLookAtLH(position, lookat, up);
}

void JWSystemCamera::SetCurrentCameraPosition(XMFLOAT3 Position) noexcept
{
	const auto& type = m_pCurrentCamera->Type;

	auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
	auto& position = transform->Position = XMVectorSet(Position.x, Position.y, Position.z, 1);
	auto& lookat = m_pCurrentCamera->LookAt;
	const auto& orientation = transform->Orientation;

	if (type == ECameraType::ThirdPerson)
	{
		lookat = XMVectorSet(Position.x, Position.y, Position.z, 0);

		const auto& zoom = m_pCurrentCamera->Zoom;
		position = lookat - orientation * zoom;
	}
	else
	{
		position = XMVectorSet(Position.x, Position.y, Position.z, 0);

		lookat = position + orientation;
	}

	UpdateCurrentCameraViewMatrix();
}

auto JWSystemCamera::GetCurrentCameraPosition() noexcept->const XMVECTOR&
{
	return m_pCurrentCamera->PtrEntity->GetComponentTransform()->Position;
}

void JWSystemCamera::Execute() noexcept
{
	if (m_pCurrentCamera == nullptr)
	{
		JW_ERROR_ABORT("메인 카메라를 지정하지 않았습니다. JWSystemCamera::SetMainCamera()를 호출하십시오.");
	}

	if (m_AreCamerasSet == false)
	{
		for (auto& iter : m_vpComponents)
		{
			if (iter)
			{
				auto transform = m_pCurrentCamera->PtrEntity->GetComponentTransform();
				const auto& orientation = transform->Orientation;

				auto& default_forward = m_pCurrentCamera->DefaultForward;
				default_forward = orientation;
			}
		}

		m_AreCamerasSet = true;
	}
}
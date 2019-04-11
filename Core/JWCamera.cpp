#include "JWCamera.h"
#include "../Core/JWDX.h"

using namespace JWEngine;

void JWCamera::Create(JWDX& DX) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	m_pDX = &DX;

	// Set default camera information
	m_CameraUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_CameraPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	m_CameraLookAt = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	m_CameraDefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	m_CameraDefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	
	// Get window size in float
	float width = static_cast<float>(m_pDX->GetWindowSize().Width);
	float height = static_cast<float>(m_pDX->GetWindowSize().Height);
	
	// Get projection matrix
	m_MatrixProjection = XMMatrixPerspectiveFovLH(KFOV * XM_PI, width / height, KNearZ, KFarZ);

	// Get orthographic matrix
	m_MatrixOrthographicFixed = XMMatrixOrthographicLH(width, height, KNearZ, KFarZ);
	m_MatrixOrthographicTransformed = m_MatrixOrthographicFixed;

	m_IsValid = true;
}

void JWCamera::MoveCamera(ECameraMoveDirection Direction, float Stride) noexcept
{
	Stride = max(Stride, 0);

	XMVECTOR camera_forward = m_CameraForward;
	XMVECTOR camera_right = m_CameraRight;
	
	switch (m_CameraType)
	{
	case JWEngine::ECameraType::FirstPerson:
		camera_forward = GetFirstPersonForward();
		camera_right = GetFirstPersonRight();
		Stride *= KFactor;
		break;
	case JWEngine::ECameraType::ThirdPerson:
		UpdateCamera();
		Stride *= KFactor;
		return;
		//break;
	case JWEngine::ECameraType::FreeLook:
		Stride *= KFactor;
		break;
	case JWEngine::ECameraType::Camera2D:
		Stride *= KFactor2D;
		break;
	default:
		break;
	}

	if (m_CameraType != JWEngine::ECameraType::Camera2D)
	{
		switch (Direction)
		{
		case JWEngine::ECameraMoveDirection::Left:
			m_CameraPosition -= Stride * camera_right;
			break;
		case JWEngine::ECameraMoveDirection::Right:
			m_CameraPosition += Stride * camera_right;
			break;
		case JWEngine::ECameraMoveDirection::Forward:
			m_CameraPosition += Stride * camera_forward;
			break;
		case JWEngine::ECameraMoveDirection::Backward:
			m_CameraPosition -= Stride * camera_forward;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (Direction)
		{
		case JWEngine::ECameraMoveDirection::Left:
			m_Camera2DPosition.x -= Stride;
			break;
		case JWEngine::ECameraMoveDirection::Right:
			m_Camera2DPosition.x += Stride;
			break;
		case JWEngine::ECameraMoveDirection::Up2D:
			m_Camera2DPosition.y -= Stride;
			break;
		case JWEngine::ECameraMoveDirection::Down2D:
			m_Camera2DPosition.y += Stride;
			break;
		default:
			break;
		}
	}

	UpdateCamera();
}

PRIVATE inline auto JWCamera::GetFirstPersonForward() noexcept->XMVECTOR
{
	XMMATRIX camera_rotation_matrix = XMMatrixRotationRollPitchYaw(0, m_Yaw, 0);
	return XMVector3TransformCoord(m_CameraDefaultForward, camera_rotation_matrix);
}

PRIVATE inline auto JWCamera::GetFirstPersonRight() noexcept->XMVECTOR
{
	XMMATRIX camera_rotation_matrix = XMMatrixRotationRollPitchYaw(0, m_Yaw, 0);
	return XMVector3TransformCoord(m_CameraDefaultRight, camera_rotation_matrix);
}

void JWCamera::RotateCamera(float PitchStride, float YawStride, float RollStride) noexcept
{
	if (m_CameraType == ECameraType::ThirdPerson)
	{
		m_Pitch -= PitchStride * KFactor;
	}
	else
	{
		m_Pitch += PitchStride * KFactor;
	}
	
	m_Yaw += YawStride * KFactor;
	m_Roll += RollStride * KFactor;

	m_Pitch = min(m_Pitch, +XM_PIDIV2 - 0.01f);
	m_Pitch = max(m_Pitch, -XM_PIDIV2 + 0.01f);

	UpdateCamera();
}

void JWCamera::ZoomCamera(float Factor) noexcept
{
	if (m_CameraType == ECameraType::ThirdPerson)
	{
		Factor *= -KFactor * 0.1f;

		XMVECTOR LookDistance = XMVector3Length(m_CameraPosition - m_CameraLookAt);
		LookDistance *= (1.0f + Factor);
		m_CameraPosition = XMVector3Normalize(m_CameraForward) * LookDistance;

		UpdateCamera();
	}
}

PRIVATE void JWCamera::UpdateCamera() noexcept
{
	switch (m_CameraType)
	{
	case JWEngine::ECameraType::FirstPerson:
		UpdateFirstPersonOrFreeLookCamera();
		break;
	case JWEngine::ECameraType::ThirdPerson:
		UpdateThirdPersonCamera();
		break;
	case JWEngine::ECameraType::FreeLook:
		UpdateFirstPersonOrFreeLookCamera();
		break;
	case JWEngine::ECameraType::Camera2D:
		Update2DCamera();
		break;
	default:
		break;
	}

	// Update float3, float4 CameraPosition as well
	XMStoreFloat4(&m_CameraPosition4, m_CameraPosition);
	XMStoreFloat3(&m_CameraPosition3, m_CameraPosition);
}

PRIVATE void JWCamera::UpdateFirstPersonOrFreeLookCamera() noexcept
{
	XMMATRIX camera_rotation_matrix = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, m_Roll);

	m_CameraForward = XMVector3TransformCoord(m_CameraDefaultForward, camera_rotation_matrix);
	m_CameraRight = XMVector3TransformCoord(m_CameraDefaultRight, camera_rotation_matrix);

	m_CameraLookAt = XMVector3Normalize(m_CameraForward);
	m_CameraLookAt = m_CameraPosition + m_CameraLookAt;
}

PRIVATE void JWCamera::UpdateThirdPersonCamera() noexcept
{
	XMMATRIX camera_rotation_matrix = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, m_Roll);

	m_CameraForward = XMVector3TransformCoord(m_CameraDefaultForward, camera_rotation_matrix);
	m_CameraRight = XMVector3TransformCoord(m_CameraDefaultRight, camera_rotation_matrix);

	XMVECTOR LookDistance = XMVector3Length(m_CameraPosition - m_CameraLookAt);
	m_CameraPosition = XMVector3Normalize(m_CameraForward) * LookDistance;
}

PRIVATE void JWCamera::Update2DCamera() noexcept
{
	m_MatrixOrthographicTransformed = m_MatrixOrthographicFixed * XMMatrixTranslation(m_Camera2DPosition.x, -m_Camera2DPosition.y, 0);
}

auto JWCamera::SetCameraType(ECameraType Type) noexcept->JWCamera&
{
	m_CameraType = Type;
	return *this;
}

auto JWCamera::SetPosition(XMFLOAT3 Position) noexcept->JWCamera&
{
	m_CameraPosition = XMVectorSet(Position.x, Position.y, Position.z, 0);
	return *this;
}

auto JWCamera::SetLookAt(XMFLOAT3 LookAt) noexcept->JWCamera&
{
	m_CameraLookAt = XMVectorSet(LookAt.x, LookAt.y, LookAt.z, 0);
	return *this;
}

auto JWCamera::GetPosition() const noexcept->const XMVECTOR&
{
	return m_CameraPosition;
}

auto JWCamera::GetPositionFloat4() const noexcept->const XMFLOAT4&
{
	return m_CameraPosition4;
}

auto JWCamera::GetPositionFloat3() const noexcept->const XMFLOAT3&
{
	return m_CameraPosition3;
}

auto JWCamera::GetViewProjectionMatrix() const noexcept->XMMATRIX
{
	// Update view matrix
	m_MatrixView = XMMatrixLookAtLH(m_CameraPosition, m_CameraLookAt, m_CameraUp);

	return m_MatrixView * m_MatrixProjection;
}

auto JWCamera::GetFixedOrthographicMatrix() const noexcept->XMMATRIX
{
	// @important: Must transpose the matrix
	return XMMatrixTranspose(m_MatrixOrthographicFixed);
}

auto JWCamera::GetTransformedOrthographicMatrix() const noexcept->XMMATRIX
{
	// @important: Must transpose the matrix
	return XMMatrixTranspose(m_MatrixOrthographicTransformed);
}
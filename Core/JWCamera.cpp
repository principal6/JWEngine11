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
	m_CameraLookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	m_CameraDefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	m_CameraDefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	// Get window size in float
	float width = static_cast<float>(m_pDX->GetWindowSize().Width);
	float height = static_cast<float>(m_pDX->GetWindowSize().Height);

	// Get projection matrix
	m_MatrixProjection = XMMatrixPerspectiveFovLH(0.4f * XM_PI, width / height, KNearZ, KFarZ);

	// Get orthographic matrix
	m_MatrixOrthographic = XMMatrixOrthographicLH(width, height, KNearZ, KFarZ);

	m_IsValid = true;
}

void JWCamera::MoveCamera(ECameraMoveDirection Direction, float Stride) noexcept
{
	Stride = max(Stride, 0);
	Stride *= KFactor;

	XMVECTOR camera_forward = m_CameraForward;
	XMVECTOR camera_right = m_CameraRight;
	
	switch (m_CameraType)
	{
	case JWEngine::ECameraType::FirstPerson:
		camera_forward = GetFirstPersonForward();
		camera_right = GetFirstPersonRight();
		break;
	case JWEngine::ECameraType::ThirdPerson:
		UpdateCamera();
		return;
		//break;
	case JWEngine::ECameraType::FreeLook:
		break;
	default:
		break;
	}

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

	UpdateCamera();
}

PRIVATE auto JWCamera::GetFirstPersonForward() noexcept->XMVECTOR
{
	XMMATRIX camera_rotation_matrix = XMMatrixRotationRollPitchYaw(0, m_Yaw, 0);
	return XMVector3TransformCoord(m_CameraDefaultForward, camera_rotation_matrix);
}

PRIVATE auto JWCamera::GetFirstPersonRight() noexcept->XMVECTOR
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
	default:
		break;
	}
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

auto JWCamera::GetPosition() const noexcept->XMVECTOR
{
	return m_CameraPosition;
}

auto JWCamera::GetViewMatrix() const noexcept->XMMATRIX
{
	// Update view matrix
	m_MatrixView = XMMatrixLookAtLH(m_CameraPosition, m_CameraLookAt, m_CameraUp);

	return m_MatrixView;
}

auto JWCamera::GetProjectionMatrix() const noexcept->XMMATRIX
{
	return m_MatrixProjection;
}

auto JWCamera::GetViewProjectionMatrix() const noexcept->XMMATRIX
{
	return GetViewMatrix() * m_MatrixProjection;
}

auto JWCamera::GetOrthographicMatrix() const noexcept->XMMATRIX
{
	return m_MatrixOrthographic;
}

auto JWCamera::GetViewOrthographicMatrix() const noexcept->XMMATRIX
{
	return GetViewMatrix() * m_MatrixOrthographic;
}
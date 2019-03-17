#include "JWCamera.h"
#include "../Core/JWDX.h"

using namespace JWEngine;

void JWCamera::Create(JWDX& DX) noexcept
{
	m_pDX = &DX;

	// Set default camera information
	m_CameraUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_CameraPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	m_CameraLookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	float width = static_cast<float>(m_pDX->GetWindowSize().Width);
	float height = static_cast<float>(m_pDX->GetWindowSize().Height);

	// Get projection matrix
	m_MatrixProjection = XMMatrixPerspectiveFovLH(0.4f * XM_PI, width / height, NEAR_Z, FAR_Z);

	// Get orthographic matrix
	m_MatrixOrthographic = XMMatrixOrthographicLH(width, height, NEAR_Z, FAR_Z);
}

void JWCamera::SetPosition(XMFLOAT4 Position) noexcept
{
	m_CameraPosition = XMVectorSet(Position.x, Position.y, Position.z, Position.w);
}
void JWCamera::SetLookAt(XMFLOAT4 LookAt) noexcept
{
	m_CameraLookAt = XMVectorSet(LookAt.x, LookAt.y, LookAt.z, LookAt.w);
}

auto JWCamera::GetPosition() noexcept->XMVECTOR
{
	return m_CameraPosition;
}

auto JWCamera::GetViewProjectionMatrix() const noexcept->XMMATRIX
{
	// Update view matrix
	m_MatrixView = XMMatrixLookAtLH(m_CameraPosition, m_CameraLookAt, m_CameraUp);

	return m_MatrixView * m_MatrixProjection;
}

auto JWCamera::GetOrthographicMatrix() const noexcept->XMMATRIX
{
	return m_MatrixOrthographic;
}
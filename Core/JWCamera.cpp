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
}

void JWCamera::SetPosition(XMFLOAT4 Position) noexcept
{
	m_CameraPosition = XMVectorSet(Position.x, Position.y, Position.z, Position.w);
}
void JWCamera::SetLookAt(XMFLOAT4 LookAt) noexcept
{
	m_CameraLookAt = XMVectorSet(LookAt.x, LookAt.y, LookAt.z, LookAt.w);
}

auto JWCamera::GetViewProjectionMatrix() const noexcept->XMMATRIX
{
	// Update view matrix
	m_MatrixView = XMMatrixLookAtLH(m_CameraPosition, m_CameraLookAt, m_CameraUp);

	// Update projection matrix
	float width = static_cast<float>(m_pDX->GetWindowSize().Width);
	float height = static_cast<float>(m_pDX->GetWindowSize().Height);
	m_MatrixProjection = XMMatrixPerspectiveFovLH(0.4f * XM_PI, width / height, 1.0f, 1000.0f);

	return m_MatrixView * m_MatrixProjection;
}
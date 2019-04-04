#include "JWImage.h"
#include "JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWImage::~JWImage()
{
	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWImage::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set member pointers.
	m_pDX = &DX;
	m_pCamera = &Camera;

	m_VertexData.Vertices.push_back(SStaticModelVertex(0, 0, 0, 0, 0));
	m_VertexData.Vertices.push_back(SStaticModelVertex(1, 0, 0, 1, 0));
	m_VertexData.Vertices.push_back(SStaticModelVertex(0, -1, 0, 0, 1));
	m_VertexData.Vertices.push_back(SStaticModelVertex(1, -1, 0, 1, 1));

	m_IndexData.Indices.push_back(SIndex3(0, 1, 2));
	m_IndexData.Indices.push_back(SIndex3(1, 3, 2));

	// Create vertex buffer
	m_pDX->CreateDynamicVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);

	m_IsValid = true;
}

void JWImage::SetPosition(XMFLOAT2 Position) noexcept
{
	m_Position = Position;

	UpdateScreenPositionAndSize();
}

void JWImage::SetSize(XMFLOAT2 Size) noexcept
{
	m_Size = Size;

	UpdateScreenPositionAndSize();
}

PRIVATE void JWImage::UpdateScreenPositionAndSize() noexcept
{
	float window_width = static_cast<float>(m_pDX->GetWindowSize().Width);
	float window_height = static_cast<float>(m_pDX->GetWindowSize().Height);

	m_VertexData.Vertices[0].Position.x = -window_width/2 + m_Position.x;
	m_VertexData.Vertices[0].Position.y = window_height/2 - m_Position.y;

	m_VertexData.Vertices[1].Position.x = m_VertexData.Vertices[0].Position.x + m_Size.x;
	m_VertexData.Vertices[1].Position.y = m_VertexData.Vertices[0].Position.y;

	m_VertexData.Vertices[2].Position.x = m_VertexData.Vertices[0].Position.x;
	m_VertexData.Vertices[2].Position.y = m_VertexData.Vertices[0].Position.y - m_Size.y;

	m_VertexData.Vertices[3].Position.x = m_VertexData.Vertices[0].Position.x + m_Size.x;
	m_VertexData.Vertices[3].Position.y = m_VertexData.Vertices[0].Position.y - m_Size.y;

	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
	{
		memcpy(mapped_subresource.pData, m_VertexData.GetPtrData(), m_VertexData.GetByteSize());

		m_pDX->GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	}
}
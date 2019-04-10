#include "JWImage.h"
#include "JWDX.h"

using namespace JWEngine;

void JWImage::Create(JWDX& DX) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set member pointers.
	m_pDX = &DX;

	m_VertexData.vVertices.push_back(SVertexStaticModel(0, 0, 0, 0, 0));
	m_VertexData.vVertices.push_back(SVertexStaticModel(1, 0, 0, 1, 0));
	m_VertexData.vVertices.push_back(SVertexStaticModel(0, -1, 0, 0, 1));
	m_VertexData.vVertices.push_back(SVertexStaticModel(1, -1, 0, 1, 1));

	m_IndexData.vIndices.push_back(SIndexTriangle(0, 1, 2));
	m_IndexData.vIndices.push_back(SIndexTriangle(1, 3, 2));

	// Create vertex buffer
	m_pDX->CreateDynamicVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);

	m_IsValid = true;
}

void JWImage::Destroy() noexcept
{
	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

auto JWImage::SetPosition(XMFLOAT2 Position) noexcept->JWImage*
{
	m_Position = Position;

	UpdateScreenPositionAndSize();

	return this;
}

auto JWImage::SetSize(XMFLOAT2 Size) noexcept->JWImage*
{
	m_Size = Size;

	UpdateScreenPositionAndSize();

	return this;
}

PRIVATE void JWImage::UpdateScreenPositionAndSize() noexcept
{
	float window_width = static_cast<float>(m_pDX->GetWindowSize().Width);
	float window_height = static_cast<float>(m_pDX->GetWindowSize().Height);

	m_VertexData.vVertices[0].Position.x = -window_width/2 + m_Position.x;
	m_VertexData.vVertices[0].Position.y = window_height/2 - m_Position.y;

	m_VertexData.vVertices[1].Position.x = m_VertexData.vVertices[0].Position.x + m_Size.x;
	m_VertexData.vVertices[1].Position.y = m_VertexData.vVertices[0].Position.y;

	m_VertexData.vVertices[2].Position.x = m_VertexData.vVertices[0].Position.x;
	m_VertexData.vVertices[2].Position.y = m_VertexData.vVertices[0].Position.y - m_Size.y;

	m_VertexData.vVertices[3].Position.x = m_VertexData.vVertices[0].Position.x + m_Size.x;
	m_VertexData.vVertices[3].Position.y = m_VertexData.vVertices[0].Position.y - m_Size.y;

	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
	{
		memcpy(mapped_subresource.pData, m_VertexData.GetPtrData(), m_VertexData.GetByteSize());

		m_pDX->GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	}
}
#include "JWImage.h"
#include "JWDX.h"

using namespace JWEngine;

void JWImage::Create(JWDX& DX, const SSize2& WindowSize) noexcept
{
	if (!m_IsCreated)
	{
		m_pDX = &DX;

		m_pWindowSize = &WindowSize;

		m_VertexData.AddVertex(SVertexModel(0, 0, 0, 0, 0));
		m_VertexData.AddVertex(SVertexModel(1, 0, 0, 1, 0));
		m_VertexData.AddVertex(SVertexModel(0, -1, 0, 0, 1));
		m_VertexData.AddVertex(SVertexModel(1, -1, 0, 1, 1));

		m_IndexData.vFaces.push_back(SIndexTriangle(0, 1, 2));
		m_IndexData.vFaces.push_back(SIndexTriangle(1, 3, 2));

		// Create vertex buffer
		m_pDX->CreateDynamicVertexBuffer(m_VertexData.GetVertexModelByteSize(), m_VertexData.GetVertexModelPtrData(), &m_VertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);

		m_IsCreated = true;
	}
}

void JWImage::Destroy() noexcept
{
	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

auto JWImage::SetPosition(XMFLOAT2 Position) noexcept->JWImage*
{
	m_Position = Position;

	UpdatePositionAndSize();

	return this;
}

auto JWImage::SetSize(XMFLOAT2 Size) noexcept->JWImage*
{
	m_Size = Size;

	UpdatePositionAndSize();

	return this;
}

auto JWImage::UpdatePositionAndSize() noexcept->JWImage*
{
	auto window_width = m_pWindowSize->floatX();
	auto window_height = m_pWindowSize->floatY();

	auto x_0 = -window_width / 2 + m_Position.x;
	auto y_0 = window_height / 2 - m_Position.y;

	m_VertexData.vVerticesModel[0].Position = XMVectorSet(x_0, y_0, 0, 0);
	m_VertexData.vVerticesModel[1].Position = XMVectorSet(x_0 + m_Size.x, y_0, 0, 0);
	m_VertexData.vVerticesModel[2].Position = XMVectorSet(x_0, y_0 - m_Size.y, 0, 0);
	m_VertexData.vVerticesModel[3].Position = XMVectorSet(x_0 + m_Size.x, y_0 - m_Size.y, 0, 0);
	
	m_pDX->UpdateDynamicResource(m_VertexBuffer, m_VertexData.GetVertexModelPtrData(), m_VertexData.GetVertexModelByteSize());

	return this;
}
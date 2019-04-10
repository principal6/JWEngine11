#include "JWLine.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWLine::~JWLine()
{
	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWLine::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	m_IsValid = true;
}

PRIVATE void JWLine::CheckValidity() const noexcept
{
	if (!m_IsValid)
	{
		JWAbort("JWLine object not valid. You must call JWLine::Create() first");
	}
}

void JWLine::AddLine(SLineRawData LineData) noexcept
{
	float window_width = static_cast<float>(m_pDX->GetWindowSize().Width);
	float window_height = static_cast<float>(m_pDX->GetWindowSize().Height);

	XMFLOAT3 position_a = XMFLOAT3(-window_width / 2 + LineData.StartPosition.x, window_height / 2 - LineData.StartPosition.y, 0);
	XMFLOAT3 position_b = position_a;
	position_b.x += LineData.Length.x;
	position_b.y += LineData.Length.y;
	
	m_VertexData.vVertices.emplace_back(position_a, LineData.Color);
	m_VertexData.vVertices.emplace_back(position_b, LineData.Color);
	m_IndexData.vIndices.emplace_back(m_VertexData.GetCount() - 2, m_VertexData.GetCount() - 1);
}

void JWLine::AddEnd() noexcept
{
	// Create vertex buffer
	m_pDX->CreateDynamicVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);
}

void JWLine::SetLine(size_t LineIndex, SLineRawData LineData) noexcept
{
	if (m_VertexData.GetCount())
	{
		LineIndex = min(LineIndex, m_VertexData.GetCount() / 2 - 1);

		float window_width = static_cast<float>(m_pDX->GetWindowSize().Width);
		float window_height = static_cast<float>(m_pDX->GetWindowSize().Height);

		XMFLOAT3 position_a = XMFLOAT3(-window_width / 2 + LineData.StartPosition.x, window_height / 2 - LineData.StartPosition.y, 0);
		XMFLOAT3 position_b = position_a;
		position_b.x += LineData.Length.x;
		position_b.y += LineData.Length.y;

		m_VertexData.vVertices[LineIndex * 2].Position = position_a;
		m_VertexData.vVertices[LineIndex * 2].ColorDiffuse = LineData.Color;

		m_VertexData.vVertices[LineIndex * 2 + 1].Position = position_b;
		m_VertexData.vVertices[LineIndex * 2 + 1].ColorDiffuse = LineData.Color;
	}
}

void JWLine::UpdateLines() noexcept
{
	UpdateVertexBuffer();
}

PRIVATE void JWLine::UpdateVertexBuffer() noexcept
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
	{
		memcpy(mapped_subresource.pData, m_VertexData.GetPtrData(), m_VertexData.GetByteSize());

		m_pDX->GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	}
}

PRIVATE void JWLine::Update() noexcept
{
	// Disable Z-buffer for 2D drawing
	m_pDX->SetDepthStencilState(EDepthStencilState::ZDisabled);

	// Set VS & PS
	m_pDX->SetVS(EVertexShader::VSBase);
	m_pDX->SetPS(EPixelShader::PSBase);

	// Set VS constant buffer
	// set WVP matrix(, which in reality is WO matrix,)
	// and send it to the constant buffer for vertex shader
	m_VSCBSpace.WVP = XMMatrixIdentity() * m_pCamera->GetFixedOrthographicMatrix();
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Set PS constant buffer
	m_pDX->UpdatePSCBFlags(false, false);
}

void JWLine::Draw() noexcept
{
	Update();

	// Set primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, m_VertexData.GetPtrStride(), m_VertexData.GetPtrOffset());
	
	// Set index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.GetCount(), 0, 0);
}
#include "JWImageCursor.h"
#include "JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWImageCursor::~JWImageCursor()
{
	JW_RELEASE(m_TextureShaderResourceView);

	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWImageCursor::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set member pointers.
	m_pDX = &DX;
	m_pCamera = &Camera;

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

PROTECTED void JWImageCursor::CheckValidity() const noexcept
{
	if (!m_IsValid)
	{
		JWAbort("JWImageCursor object not valid. You must call JWImageCursor::Create() first");
	}
}

void JWImageCursor::LoadImageCursorFromFile(STRING Directory, STRING FileName) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsTextureCreated);

	CheckValidity();

	STRING path_string = Directory + FileName;
	WSTRING w_path = StringToWstring(path_string);

	// Texture resource
	ID3D11Texture2D* p_resource{};

	CreateWICTextureFromFile(m_pDX->GetDevice(), w_path.c_str(), (ID3D11Resource**)&p_resource, &m_TextureShaderResourceView, 0);
	
	// Get texture file info
	D3D11_TEXTURE2D_DESC texture_description{};
	p_resource->GetDesc(&texture_description);
	m_OriginalSize.Width = texture_description.Width;
	m_OriginalSize.Height = texture_description.Height;

	// Set size
	m_Size = XMFLOAT2(static_cast<float>(m_OriginalSize.Width), static_cast<float>(m_OriginalSize.Height));

	// Release the resource
	JW_RELEASE(p_resource);

	m_IsTextureCreated = true;
}

auto JWImageCursor::SetPosition(XMFLOAT2 Position) noexcept->JWImageCursor&
{
	CheckValidity();

	m_Position = Position;

	UpdateVertices();

	return *this;
}

PROTECTED void JWImageCursor::UpdateVertices() noexcept
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

void JWImageCursor::Draw() noexcept
{
	// Disable Z-buffer for 2D drawing
	m_pDX->SetDepthStencilState(EDepthStencilState::ZDisabled);

	// Set VS & PS
	m_pDX->SetVS(EVertexShader::VSBase);
	m_pDX->SetPS(EPixelShader::PSBase);

	// Update VS constant buffer (WVP matrix, which in reality is WO matrix.)
	m_VSCBSpace.WVP = XMMatrixTranspose(XMMatrixIdentity() * m_pCamera->GetTransformedOrthographicMatrix());
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Set PS texture and sampler
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->SetPSSamplerState(ESamplerState::MinMagMipLinearWrap);

	// Update PS constant buffer
	m_pDX->UpdatePSCBFlags(true, false);

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, m_VertexData.GetPtrStride(), m_VertexData.GetPtrOffset());
	
	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw indexed
	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.GetCount(), 0, 0);
}
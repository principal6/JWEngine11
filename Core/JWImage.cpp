#include "JWImage.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWImage::~JWImage()
{
	JW_RELEASE(m_TextureShaderResourceView);

	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWImage::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	AddVertex(SVertex(0, 0, 0, 0, 0));
	AddVertex(SVertex(1, 0, 0, 1, 0));
	AddVertex(SVertex(0, -1, 0, 0, 1));
	AddVertex(SVertex(1, -1, 0, 1, 1));

	AddIndex(SIndex3(0, 1, 2));
	AddIndex(SIndex3(1, 3, 2));

	AddEnd();

	m_IsValid = true;
}

PROTECTED void JWImage::CheckValidity() const noexcept
{
	if (!m_IsValid)
	{
		JWAbort("JWImage object not valid. You must call JWImage::Create() first");
	}
}

PROTECTED void JWImage::AddVertex(const SVertex& Vertex) noexcept
{
	m_VertexData.Vertices.push_back(Vertex);
}

PROTECTED void JWImage::AddIndex(const SIndex3& Index) noexcept
{
	m_IndexData.Indices.push_back(Index);
}

PROTECTED void JWImage::AddEnd() noexcept
{
	// Create vertex buffer
	m_pDX->CreateDynamicVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);
}

void JWImage::LoadImageFromFile(STRING Directory, STRING FileName) noexcept
{
	AVOID_DUPLICATE_CREATION(m_IsTextureCreated);

	CheckValidity();

	STRING path_string = Directory + FileName;
	CreateTexture(StringToWstring(path_string));
}

PROTECTED void JWImage::CreateTexture(WSTRING TextureFileName) noexcept
{
	ID3D11Texture2D* p_resource{};

	CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), (ID3D11Resource**)&p_resource, &m_TextureShaderResourceView, 0);
	
	// Get file info
	D3D11_TEXTURE2D_DESC texture_description{};
	p_resource->GetDesc(&texture_description);
	m_ImageOriginalSize.Width = texture_description.Width;
	m_ImageOriginalSize.Height = texture_description.Height;
	SetSize(XMFLOAT2(static_cast<float>(m_ImageOriginalSize.Width), static_cast<float>(m_ImageOriginalSize.Height)));

	// Release the resource
	JW_RELEASE(p_resource);

	m_IsTextureCreated = true;
}

auto JWImage::SetPosition(XMFLOAT2 Position) noexcept->JWImage&
{
	m_Position = Position;

	UpdateScreenPositionAndSize();

	return *this;
}

auto JWImage::SetSize(XMFLOAT2 Size) noexcept->JWImage&
{
	m_Size = Size;

	UpdateScreenPositionAndSize();

	return *this;
}

PROTECTED void JWImage::UpdateScreenPositionAndSize() noexcept
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

	if (m_IsValid)
	{
		UpdateVertexBuffer();
	}
}

PROTECTED void JWImage::UpdateVertexBuffer() noexcept
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
	{
		memcpy(mapped_subresource.pData, m_VertexData.GetPtrData(), m_VertexData.GetByteSize());

		m_pDX->GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	}
}

void JWImage::UpdateAll() noexcept
{
	UpdateDefaultVSConstantBuffer();
	UpdateDefaultPSConstantBuffer();
	UpdateTexture();
}

PROTECTED void JWImage::UpdateDefaultVSConstantBuffer() noexcept
{
	// Set VS constant buffer
	// set WVP matrix(, which in reality is WO matrix,)
	// and send it to the constant buffer for vertex shader
	m_WVP = XMMatrixIdentity() * m_pCamera->GetOrthographicMatrix();
	m_pDX->SetDefaultVSConstantBufferData(SDefaultVSConstantBufferData(XMMatrixTranspose(m_WVP)));
}

PROTECTED void JWImage::UpdateDefaultPSConstantBuffer() noexcept
{
	// Set PS constant buffer
	m_pDX->SetDefaultPSConstantBufferData(SDefaultPSConstantBufferData(TRUE));
}

PROTECTED void JWImage::UpdateTexture() noexcept
{
	// Set texture and sampler for pixel shader
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->SetPSSamplerState(ESamplerState::LinearWrap);
}

void JWImage::Draw() noexcept
{
	// Set primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, m_VertexData.GetPtrStride(), m_VertexData.GetPtrOffset());
	
	// Set index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.GetCount(), 0, 0);
}
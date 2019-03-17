#include "JWImage.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWImage::~JWImage()
{
	m_TextureSamplerState->Release();
	m_TextureShaderResourceView->Release();

	m_VertexBuffer->Release();
	m_IndexBuffer->Release();
}

void JWImage::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	if (m_IsValid)
	{
		// Avoid duplicate creation
		return;
	}

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	AddVertex(SVertex(0, 0, 0, 0, 0));
	AddVertex(SVertex(1, 0, 0, 1, 0));
	AddVertex(SVertex(0, -1, 0, 0, 1));
	AddVertex(SVertex(1, -1, 0, 1, 1));

	AddIndex(SIndex(0, 1, 2));
	AddIndex(SIndex(1, 3, 2));

	AddEnd();

	m_IsValid = true;
}

PRIVATE void JWImage::CheckValidity() const noexcept
{
	if (!m_IsValid)
	{
		JWAbort("JWImage2D object not valid. You must call JWImage2D::Create() first");
	}
}

PRIVATE inline void JWImage::AddVertex(const SVertex& Vertex) noexcept
{
	m_VertexData.Vertices.push_back(Vertex);
}

PRIVATE inline void JWImage::AddIndex(const SIndex& Index) noexcept
{
	m_IndexData.Indices.push_back(Index);
}

PRIVATE void JWImage::AddEnd() noexcept
{
	// Calculate the count of vertices
	m_VertexData.Count = static_cast<UINT>(m_VertexData.Vertices.size());

	// Calculate the count of indices
	m_IndexData.Count = static_cast<UINT>(m_IndexData.Indices.size() * 3);

	// Create vertex buffer
	CreateVertexBuffer();

	// Create index buffer
	CreateIndexBuffer();

	// Set the primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

PRIVATE void JWImage::CreateVertexBuffer() noexcept
{
	D3D11_BUFFER_DESC vertex_buffer_description{};
	vertex_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
	vertex_buffer_description.ByteWidth = sizeof(SVertex) * m_VertexData.Count;
	vertex_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertex_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_buffer_data{};
	vertex_buffer_data.pSysMem = &m_VertexData.Vertices[0];

	// Create vertex buffer
	m_pDX->GetDevice()->CreateBuffer(&vertex_buffer_description, &vertex_buffer_data, &m_VertexBuffer);
}

PRIVATE void JWImage::CreateIndexBuffer() noexcept
{
	D3D11_BUFFER_DESC index_buffer_description{};
	index_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_description.ByteWidth = sizeof(DWORD) * m_IndexData.Count;
	index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_description.CPUAccessFlags = 0;
	index_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA index_buffer_data{};
	index_buffer_data.pSysMem = &m_IndexData.Indices[0];

	// Create index buffer
	m_pDX->GetDevice()->CreateBuffer(&index_buffer_description, &index_buffer_data, &m_IndexBuffer);
}

void JWImage::LoadImageFromFile(STRING Directory, STRING FileName) noexcept
{
	CheckValidity();

	if (m_IsTextureCreated)
	{
		// Avoid duplicate creation.
		return;
	}

	STRING path_string = Directory + FileName;
	CreateTexture(StringToWstring(path_string));
}

PRIVATE void JWImage::CreateTexture(WSTRING TextureFileName) noexcept
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
	p_resource->Release();
	p_resource = nullptr;

	CreateSamplerState();

	m_IsTextureCreated = true;
}

PRIVATE void JWImage::CreateSamplerState() noexcept
{
	if (m_TextureSamplerState)
	{
		// To avoid duplicate creation.
		return;
	}

	D3D11_SAMPLER_DESC sampler_description{};
	sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_description.MinLOD = 0;
	sampler_description.MaxLOD = D3D11_FLOAT32_MAX;
	
	m_pDX->GetDevice()->CreateSamplerState(&sampler_description, &m_TextureSamplerState);
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

	if (m_IsValid)
	{
		UpdateVertexBuffer();
	}
}

PRIVATE void JWImage::UpdateVertexBuffer() noexcept
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	m_pDX->GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);

	memcpy(mapped_subresource.pData, &m_VertexData.Vertices[0], sizeof(SVertex) * m_VertexData.Count);

	m_pDX->GetDeviceContext()->Unmap(m_VertexBuffer, 0);
}

PRIVATE void JWImage::Update() noexcept
{
	// Set WVP matrix(, which in reality is WO matrix,)
	// and send it to the constant buffer for vertex shader
	m_WVP = XMMatrixIdentity() * m_pCamera->GetOrthographicMatrix();
	m_pDX->SetConstantBufferData(SConstantBufferDataPerObject(XMMatrixTranspose(m_WVP)));

	// Set texture and sampler for pixel shader
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->GetDeviceContext()->PSSetSamplers(0, 1, &m_TextureSamplerState);
}

void JWImage::Draw() noexcept
{
	Update();

	// Set vertex buffer
	UINT vertex_stride{ sizeof(SVertex) };
	UINT vertex_offset{};
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &vertex_stride, &vertex_offset);
	
	// Set index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.Count, 0, 0);
}
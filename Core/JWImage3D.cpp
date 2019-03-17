#include "JWImage3D.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWImage3D::~JWImage3D()
{
	m_TextureSamplerState->Release();
	m_TextureShaderResourceView->Release();

	m_VertexBuffer->Release();
	m_IndexBuffer->Release();
}

void JWImage3D::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	m_MatrixWorld = XMMatrixIdentity();
	m_MatrixTranslation = XMMatrixIdentity();
	m_MatrixRotation = XMMatrixIdentity();
	m_MatrixScale = XMMatrixIdentity();

	AddVertex(SVertex(-1, -1, 0, 0, 1));
	AddVertex(SVertex(-1, +1, 0, 0, 0));
	AddVertex(SVertex(+1, +1, 0, 1, 0));
	AddVertex(SVertex(+1, -1, 0, 1, 1));

	AddIndex(SIndex(0, 1, 2));
	AddIndex(SIndex(0, 2, 3));

	AddEnd();

	m_IsValid = true;
}

PRIVATE void JWImage3D::CheckValidity() const noexcept
{
	if (!m_IsValid)
	{
		JWAbort("JWImage3D object not valid. You must call JWImage3D::Create() first");
	}
}

PRIVATE inline void JWImage3D::AddVertex(const SVertex& Vertex) noexcept
{
	m_VertexData.Vertices.push_back(Vertex);
}

PRIVATE inline void JWImage3D::AddIndex(const SIndex& Index) noexcept
{
	m_IndexData.Indices.push_back(Index);
}

PRIVATE void JWImage3D::AddEnd() noexcept
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

PRIVATE void JWImage3D::CreateVertexBuffer() noexcept
{
	D3D11_BUFFER_DESC vertex_buffer_description{};
	vertex_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_description.ByteWidth = sizeof(SVertex) * m_VertexData.Count;
	vertex_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_description.CPUAccessFlags = 0;
	vertex_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_buffer_data{};
	vertex_buffer_data.pSysMem = &m_VertexData.Vertices[0];

	// Create vertex buffer
	m_pDX->GetDevice()->CreateBuffer(&vertex_buffer_description, &vertex_buffer_data, &m_VertexBuffer);
}

PRIVATE void JWImage3D::CreateIndexBuffer() noexcept
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

void JWImage3D::LoadImageFromFile(STRING Directory, STRING FileName) noexcept
{
	CheckValidity();

	STRING path_string = Directory + FileName;
	CreateTexture(StringToWstring(path_string));
}

PRIVATE void JWImage3D::CreateTexture(WSTRING TextureFileName) noexcept
{
	CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &m_TextureShaderResourceView, 0);

	CreateSamplerState();
}

PRIVATE void JWImage3D::CreateSamplerState() noexcept
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

PRIVATE void JWImage3D::Update() noexcept
{
	// Set WVP matrix and send it to the constant buffer for vertex shader
	m_WVP = m_MatrixWorld * m_pCamera->GetViewProjectionMatrix();
	m_pDX->SetConstantBufferData(SConstantBufferDataPerObject(XMMatrixTranspose(m_WVP)));

	// Set texture and sampler for pixel shader
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->GetDeviceContext()->PSSetSamplers(0, 1, &m_TextureSamplerState);
}

void JWImage3D::Draw() noexcept
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
#include "JWImageCursor.h"
#include "JWDX.h"

using namespace JWEngine;

JWImageCursor::~JWImageCursor()
{
	JW_RELEASE(m_TextureShaderResourceView);

	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWImageCursor::Create(JWDX& DX, XMFLOAT2 WindowSize) noexcept
{
	if (!m_IsCreated)
	{
		m_pDX = &DX;
		
		// Set orthographic projection matrix.
		m_MatrixProjOrthographic = XMMatrixOrthographicLH(WindowSize.x, WindowSize.y, KOrthographicNearZ, KOrthographicFarZ);

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

void JWImageCursor::LoadImageCursorFromFile(STRING Directory, STRING FileName) noexcept
{
	if ((m_IsCreated) && (!m_IsTextureCreated))
	{
		STRING path_string = Directory + FileName;
		WSTRING w_path = StringToWstring(path_string);

		// Texture resource
		ID3D11Texture2D* p_resource{};

		CreateWICTextureFromFile(m_pDX->GetDevice(), w_path.c_str(), (ID3D11Resource * *)& p_resource, &m_TextureShaderResourceView, 0);

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
}

auto JWImageCursor::SetPosition(XMFLOAT2 Position) noexcept->JWImageCursor&
{
	if (m_IsCreated)
	{
		m_Position = Position;

		UpdateVertices();
	}

	return *this;
}

PROTECTED void JWImageCursor::UpdateVertices() noexcept
{
	float window_width = m_pDX->GetWindowSize().x;
	float window_height = m_pDX->GetWindowSize().y;

	m_VertexData.vVerticesModel[0].Position.x = -window_width/2 + m_Position.x;
	m_VertexData.vVerticesModel[0].Position.y = window_height/2 - m_Position.y;

	m_VertexData.vVerticesModel[1].Position.x = m_VertexData.vVerticesModel[0].Position.x + m_Size.x;
	m_VertexData.vVerticesModel[1].Position.y = m_VertexData.vVerticesModel[0].Position.y;

	m_VertexData.vVerticesModel[2].Position.x = m_VertexData.vVerticesModel[0].Position.x;
	m_VertexData.vVerticesModel[2].Position.y = m_VertexData.vVerticesModel[0].Position.y - m_Size.y;

	m_VertexData.vVerticesModel[3].Position.x = m_VertexData.vVerticesModel[0].Position.x + m_Size.x;
	m_VertexData.vVerticesModel[3].Position.y = m_VertexData.vVerticesModel[0].Position.y - m_Size.y;

	m_pDX->UpdateDynamicResource(m_VertexBuffer, m_VertexData.GetVertexModelPtrData(), m_VertexData.GetVertexModelByteSize());
}

void JWImageCursor::Draw() noexcept
{
	// Disable Z-buffer for 2D drawing
	m_pDX->SetDepthStencilState(EDepthStencilState::ZDisabled);

	// Set VS & PS
	m_pDX->SetVS(EVertexShader::VSBase);
	m_pDX->SetPS(EPixelShader::PSBase);

	// Update VS constant buffer (WVP matrix, which in reality is WO matrix.)
	m_VSCBSpace.WVP = XMMatrixTranspose(m_MatrixProjOrthographic);
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Set PS texture and sampler
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->SetPSSamplerState(ESamplerState::MinMagMipLinearWrap);

	// Update PS constant buffer
	m_pDX->UpdatePSCBFlags(JWFlagPS_UseDiffuseTexture);

	// Set IA primitive topology
	m_pDX->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, m_VertexData.GetPtrStrides(), m_VertexData.GetPtrOffsets());
	
	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw indexed
	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.GetCount(), 0, 0);
}
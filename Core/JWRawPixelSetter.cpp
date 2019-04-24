#include "JWRawPixelSetter.h"
#include "JWDX.h"

using namespace JWEngine;

void JWRawPixelSetter::Create(JWDX& DX) noexcept
{
	if (!m_IsCreated)
	{
		// Set JWDX pointer.
		m_pDX = &DX;

		// Create raw pixel data
		m_RawPixelData.CreatePixelData(m_pDX->GetWindowSize());

		// Set default color
		for (uint32_t y = 0; y < m_RawPixelData.GetHeight(); ++y)
		{
			for (uint32_t x = 0; x < m_RawPixelData.GetWidth(); ++x)
			{
				m_RawPixelData.SetPixel(x, y, SRawPixelColor(255, 255, 0, 255));
			}
		}

		CreateRawTexture();

		m_IsCreated = true;
	}
}

PRIVATE void JWRawPixelSetter::CreateRawTexture() noexcept
{
	DXGI_FORMAT texture_format = DXGI_FORMAT_B8G8R8A8_UNORM; // DXGI_FORMAT_B8G8R8A8_UNORM?? DXGI_FORMAT_R32G32B32A32_FLOAT??

	D3D11_TEXTURE2D_DESC texture_descrption{};
	texture_descrption.Width = m_RawPixelData.GetWidth();
	texture_descrption.Height = m_RawPixelData.GetHeight();
	texture_descrption.MipLevels = 1;
	texture_descrption.ArraySize = 1;
	texture_descrption.Format = texture_format;
	texture_descrption.SampleDesc.Count = 1;
	texture_descrption.Usage = D3D11_USAGE_DYNAMIC;
	texture_descrption.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture_descrption.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texture_descrption.MiscFlags = 0;

	// Create the texture
	if (SUCCEEDED(m_pDX->GetDevice()->CreateTexture2D(&texture_descrption, nullptr, &m_RawTexture2D)))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_description{};
		srv_description.Format = texture_format;
		srv_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_description.Texture2D.MostDetailedMip = 0;
		srv_description.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		m_pDX->GetDevice()->CreateShaderResourceView(m_RawTexture2D, &srv_description, &m_RawTexture2DSRV);
	}
}

void JWRawPixelSetter::Destroy() noexcept
{
	JW_RELEASE(m_RawTexture2DSRV);
	JW_RELEASE(m_RawTexture2D);
}

auto JWRawPixelSetter::GetRawPixelData() noexcept->SRawPixelData&
{
	return m_RawPixelData;
}

void JWRawPixelSetter::Draw() noexcept
{
	UpdateRawTexture();

	// Disable Z-buffer for 2D drawing
	// in order to draw them on top of everything else
	m_pDX->SetDepthStencilState(EDepthStencilState::ZDisabled);

	// Set default VS & PS
	m_pDX->SetVS(EVertexShader::VSRaw);
	m_pDX->SetPS(EPixelShader::PSRaw);

	// Set texture and sampler for pixel shader (RawPixelSetter = t0)
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_RawTexture2DSRV);
	m_pDX->SetPSSamplerState(ESamplerState::MinMagMipPointWrap);

	// Set primitive topology
	m_pDX->SetPrimitiveTopology(EPrimitiveTopology::TriangleStrip);

	// Draw Screen-quad
	m_pDX->GetDeviceContext()->IASetInputLayout(nullptr);
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	m_pDX->GetDeviceContext()->Draw(4, 0);
}

PRIVATE void JWRawPixelSetter::UpdateRawTexture() noexcept
{
	m_pDX->UpdateDynamicResource(m_RawTexture2D, m_RawPixelData.GetPtrData(), m_RawPixelData.GetByteSize());
}
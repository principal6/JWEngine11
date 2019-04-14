#include "JWRawPixelSetter.h"
#include "../Core/JWDX.h"

using namespace JWEngine;

JWRawPixelSetter::~JWRawPixelSetter()
{
	JW_RELEASE(m_RawTexture2DSRV);
	JW_RELEASE(m_RawTexture2D);
}

void JWRawPixelSetter::Create(JWDX& DX) noexcept
{
	assert(!m_IsCreated);

	// Set JWDX pointer.
	m_pDX = &DX;

	// Create raw pixel data
	m_RawPixelData.CreatePixelData(m_pDX->GetWindowSize().Width, m_pDX->GetWindowSize().Height);

	// Set default color
	for (int y = 0; y < m_RawPixelData.GetHeight(); ++y)
	{
		for (int x = 0; x < m_RawPixelData.GetWidth(); ++x)
		{
			m_RawPixelData.SetPixel(x, y, SRawPixelColor(255, 255, 0, 255));
		}
	}

	CreateRawTexture();

	m_IsCreated = true;
}

void JWRawPixelSetter::CreateRawTexture() noexcept
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
	m_pDX->GetDevice()->CreateTexture2D(&texture_descrption, nullptr, &m_RawTexture2D);

	assert(m_RawTexture2D);

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_description{};
	srv_description.Format = texture_format;
	srv_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_description.Texture2D.MostDetailedMip = 0;
	srv_description.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	m_pDX->GetDevice()->CreateShaderResourceView(m_RawTexture2D, &srv_description, &m_RawTexture2DSRV);
}

auto JWRawPixelSetter::GetRawPixelData() noexcept->SRawPixelData&
{
	return m_RawPixelData;
}

void JWRawPixelSetter::UpdateRawTexture() noexcept
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(m_RawTexture2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
	{
		memcpy(mapped_subresource.pData, m_RawPixelData.GetPtrData(), m_RawPixelData.GetByteSize());

		m_pDX->GetDeviceContext()->Unmap(m_RawTexture2D, 0);
	}
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
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Draw Screen-quad
	m_pDX->GetDeviceContext()->IASetInputLayout(nullptr);
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	m_pDX->GetDeviceContext()->Draw(4, 0);
}

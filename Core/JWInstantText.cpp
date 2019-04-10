#include "JWInstantText.h"
#include "JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWInstantText::~JWInstantText()
{
	JW_RELEASE(m_TextureShaderResourceView);

	JW_RELEASE(m_PSInstantTextCB);
	JW_RELEASE(m_PSInstantText);
	
	JW_RELEASE(m_IndexBuffer);
	JW_RELEASE(m_VertexBuffer);
}

void JWInstantText::Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory, STRING FontFileName) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	m_BaseDirectory = BaseDirectory;

	if (!m_FontParser.IsParsed())
	{
		// Font is not yet parsed, then parse it.
		// The font data(ms_FontData) is static, so we can run this code only once per process,
		// no matter how many JWInstantTexts or windows are made.

		if (!m_FontParser.Parse(StringToWstring(BaseDirectory + FontFileName + ".fnt")))
		{
			JWAbort("Unable to parse the font data from file. Please check the file name.");
		}
	}

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	CreateInstantTextVertexBuffer();
	CreateInstantTextIndexBuffer();
	CreateInstantTextPS();
	CreatePSConstantBuffer();

	m_IsValid = true;

	LoadImageFromFile(BaseDirectory, FontFileName + "_0.png");
}

PRIVATE void JWInstantText::CreateInstantTextVertexBuffer() noexcept
{
	m_VertexData.vVertices.clear();
	m_VertexData.vVertices.reserve(KMaxInsantTextLength * 4);

	for (int iterator_count = 0; iterator_count < KMaxInsantTextLength * 4; ++iterator_count)
	{
		m_VertexData.vVertices.push_back(SVertexStaticModel());
	}

	// Create vertex buffer
	m_pDX->CreateDynamicVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);
}

PRIVATE void JWInstantText::CreateInstantTextIndexBuffer() noexcept
{
	m_IndexData.vIndices.clear();
	m_IndexData.vIndices.reserve(KMaxInsantTextLength * 2);

	for (int iterator_count = 0; iterator_count < KMaxInsantTextLength; ++iterator_count)
	{
		m_IndexData.vIndices.push_back(SIndexTriangle(iterator_count * 4,     iterator_count * 4 + 1, iterator_count * 4 + 2));
		m_IndexData.vIndices.push_back(SIndexTriangle(iterator_count * 4 + 1, iterator_count * 4 + 3, iterator_count * 4 + 2));
	}

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);
}

PRIVATE void JWInstantText::CreateInstantTextPS() noexcept
{
	ID3D10Blob* buffer_ps{};

	// Compile Shaders from shader file
	WSTRING shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\PSInstantText.hlsl";
	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0", 0, 0, &buffer_ps, nullptr);

	// Create the shader
	m_pDX->GetDevice()->CreatePixelShader(buffer_ps->GetBufferPointer(), buffer_ps->GetBufferSize(), nullptr, &m_PSInstantText);

	JW_RELEASE(buffer_ps);
}

PRIVATE void JWInstantText::CreatePSConstantBuffer() noexcept
{
	// Create buffer to send to constant buffer in HLSL
	D3D11_BUFFER_DESC constant_buffer_description{};
	constant_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_description.ByteWidth = sizeof(SPSInstantTextCBData);
	constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constant_buffer_description.CPUAccessFlags = 0;
	constant_buffer_description.MiscFlags = 0;

	m_pDX->GetDevice()->CreateBuffer(&constant_buffer_description, nullptr, &m_PSInstantTextCB);
}

PRIVATE void JWInstantText::LoadImageFromFile(STRING Directory, STRING FileName) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsTextureCreated);

	STRING path_string = Directory + FileName;
	WSTRING w_path = StringToWstring(path_string);

	CreateWICTextureFromFile(m_pDX->GetDevice(), w_path.c_str(), nullptr, &m_TextureShaderResourceView, 0);

	m_IsTextureCreated = true;
}

void JWInstantText::BeginRendering() noexcept
{
	// Set rasterizer state
	m_pDX->SetRasterizerState(ERasterizerState::SolidNoCull);

	// Set blend state
	m_pDX->SetBlendState(EBlendState::Transprent);

	// Disable Z-buffer for 2D drawing
	m_pDX->SetDepthStencilState(EDepthStencilState::ZDisabled);

	// Set VS
	m_pDX->SetVS(EVertexShader::VSBase);

	// Update VS constant buffer (WVP matrix, which in reality is WO matrix.)
	m_VSCBSpace.WVP = XMMatrixIdentity() * m_pCamera->GetTransformedOrthographicMatrix();
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Set PS
	m_pDX->GetDeviceContext()->PSSetShader(m_PSInstantText, nullptr, 0);

	// Set PS texture and sampler
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->SetPSSamplerState(ESamplerState::MinMagMipLinearWrap);

	// Empty the vertex data
	m_VertexData.EmptyData();

	// Initialize text length
	m_CurrentTextLength = 0;
}

void JWInstantText::RenderText(STRING Text, XMFLOAT2 Position, XMFLOAT4 FontColorRGB) noexcept
{
	float window_width_half = static_cast<float>(m_pDX->GetWindowSize().Width) / 2;
	float window_height_half = static_cast<float>(m_pDX->GetWindowSize().Height) / 2;
	float texture_width = m_FontParser.GetFontTextureWidth();
	float texture_height = m_FontParser.GetFontTextureHeight();
	float base_x_position = -window_width_half + Position.x;
	float base_y_position = window_height_half - Position.y;
	float x1{}, x2{}, y1{}, y2{};
	float u1{}, u2{}, v1{}, v2{};
	BMFont::BMChar current_bm_char{};

	WSTRING wide_text = StringToWstring(Text);
	size_t iterator_index{ m_CurrentTextLength * 4 };
	for (auto iterator_char : wide_text)
	{
		current_bm_char = m_FontParser.GetBMCharFromWideCharacter(iterator_char);

		u1 = static_cast<float>(current_bm_char.X) / texture_width;
		v1 = static_cast<float>(current_bm_char.Y) / texture_height;
		u2 = u1 + static_cast<float>(current_bm_char.Width) / texture_width;
		v2 = v1 + static_cast<float>(current_bm_char.Height) / texture_height;

		x1 = base_x_position + static_cast<float>(current_bm_char.XOffset);
		y1 = base_y_position - static_cast<float>(current_bm_char.YOffset);
		x2 = x1 + static_cast<float>(current_bm_char.Width);
		y2 = y1 - static_cast<float>(current_bm_char.Height);

		m_VertexData.vVertices[iterator_index * 4].Position.x = x1;
		m_VertexData.vVertices[iterator_index * 4].Position.y = y1;
		m_VertexData.vVertices[iterator_index * 4].TextureCoordinates.x = u1;
		m_VertexData.vVertices[iterator_index * 4].TextureCoordinates.y = v1;
		m_VertexData.vVertices[iterator_index * 4].ColorDiffuse = FontColorRGB;

		m_VertexData.vVertices[iterator_index * 4 + 1].Position.x = x2;
		m_VertexData.vVertices[iterator_index * 4 + 1].Position.y = y1;
		m_VertexData.vVertices[iterator_index * 4 + 1].TextureCoordinates.x = u2;
		m_VertexData.vVertices[iterator_index * 4 + 1].TextureCoordinates.y = v1;
		m_VertexData.vVertices[iterator_index * 4 + 1].ColorDiffuse = FontColorRGB;

		m_VertexData.vVertices[iterator_index * 4 + 2].Position.x = x1;
		m_VertexData.vVertices[iterator_index * 4 + 2].Position.y = y2;
		m_VertexData.vVertices[iterator_index * 4 + 2].TextureCoordinates.x = u1;
		m_VertexData.vVertices[iterator_index * 4 + 2].TextureCoordinates.y = v2;
		m_VertexData.vVertices[iterator_index * 4 + 2].ColorDiffuse = FontColorRGB;

		m_VertexData.vVertices[iterator_index * 4 + 3].Position.x = x2;
		m_VertexData.vVertices[iterator_index * 4 + 3].Position.y = y2;
		m_VertexData.vVertices[iterator_index * 4 + 3].TextureCoordinates.x = u2;
		m_VertexData.vVertices[iterator_index * 4 + 3].TextureCoordinates.y = v2;
		m_VertexData.vVertices[iterator_index * 4 + 3].ColorDiffuse = FontColorRGB;

		base_x_position += current_bm_char.XAdvance;
		++iterator_index;
	}

	m_CurrentTextLength += wide_text.length();
}

void JWInstantText::EndRendering() noexcept
{
	// Update vertex buffer
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
	{
		memcpy(mapped_subresource.pData, m_VertexData.GetPtrData(), m_VertexData.GetByteSize());

		m_pDX->GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	}

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, m_VertexData.GetPtrStride(), m_VertexData.GetPtrOffset());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw indexed
	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.GetCount(), 0, 0);
}
#include "JWInstantText.h"
#include "JWDX.h"

using namespace JWEngine;

JWInstantText::~JWInstantText()
{
	JW_RELEASE(m_InstantTextPS);
	JW_RELEASE(m_InstantTextPSConstantBuffer);
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

	JWImage::LoadImageFromFile(BaseDirectory, FontFileName + "_0.png");
}

PRIVATE void JWInstantText::CreateInstantTextVertexBuffer() noexcept
{
	m_VertexData.Vertices.clear();
	m_VertexData.Vertices.reserve(MAX_INSTANT_TEXT_LENGTH * 4);

	for (int iterator_count = 0; iterator_count < MAX_INSTANT_TEXT_LENGTH * 4; ++iterator_count)
	{
		m_VertexData.Vertices.push_back(SVertex());
	}

	// Create vertex buffer
	m_pDX->CreateDynamicVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);
}

PRIVATE void JWInstantText::CreateInstantTextIndexBuffer() noexcept
{
	m_IndexData.Indices.clear();
	m_IndexData.Indices.reserve(MAX_INSTANT_TEXT_LENGTH * 2);

	for (int iterator_count = 0; iterator_count < MAX_INSTANT_TEXT_LENGTH; ++iterator_count)
	{
		m_IndexData.Indices.push_back(SIndex3(iterator_count * 4,     iterator_count * 4 + 1, iterator_count * 4 + 2));
		m_IndexData.Indices.push_back(SIndex3(iterator_count * 4 + 1, iterator_count * 4 + 3, iterator_count * 4 + 2));
	}

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);
}

PRIVATE void JWInstantText::CreateInstantTextPS() noexcept
{
	ID3D10Blob* buffer_ps{};

	// Compile Shaders from shader file
	WSTRING shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\InstantTextPS.hlsl";
	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0", 0, 0, &buffer_ps, nullptr);

	// Create the shader
	m_pDX->GetDevice()->CreatePixelShader(buffer_ps->GetBufferPointer(), buffer_ps->GetBufferSize(), nullptr, &m_InstantTextPS);

	JW_RELEASE(buffer_ps);
}

PRIVATE void JWInstantText::CreatePSConstantBuffer() noexcept
{
	// Create buffer to send to constant buffer in HLSL
	D3D11_BUFFER_DESC constant_buffer_description{};
	constant_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_description.ByteWidth = sizeof(SInstantTextPSConstantBufferData);
	constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constant_buffer_description.CPUAccessFlags = 0;
	constant_buffer_description.MiscFlags = 0;

	m_pDX->GetDevice()->CreateBuffer(&constant_buffer_description, nullptr, &m_InstantTextPSConstantBuffer);
}

void JWInstantText::SetInstantTextPS() noexcept
{
	m_pDX->GetDeviceContext()->PSSetShader(m_InstantTextPS, nullptr, 0);
}

void JWInstantText::DrawInstantText(STRING Text, XMFLOAT2 Position, XMFLOAT3 FontColorRGB) noexcept
{
	JWImage::UpdateDefaultVSConstantBuffer();
	JWImage::UpdateTexture();

	// Set pixel shader
	SetInstantTextPS();
	
	// Update InstantText pixel shader's constant buffer (font color)
	m_TextColor._RGBA = XMFLOAT4(FontColorRGB.x, FontColorRGB.y, FontColorRGB.z, 1);
	m_pDX->GetDeviceContext()->UpdateSubresource(m_InstantTextPSConstantBuffer, 0, nullptr, &m_TextColor, 0, 0);
	m_pDX->GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_InstantTextPSConstantBuffer);

	// Empty vertex data
	m_VertexData.EmptyData();
	
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
	size_t iterator_index{};
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

		m_VertexData.Vertices[iterator_index * 4].Position.x = x1;
		m_VertexData.Vertices[iterator_index * 4].Position.y = y1;
		m_VertexData.Vertices[iterator_index * 4].TextureCoordinates.x = u1;
		m_VertexData.Vertices[iterator_index * 4].TextureCoordinates.y = v1;
		m_VertexData.Vertices[iterator_index * 4 + 1].Position.x = x2;
		m_VertexData.Vertices[iterator_index * 4 + 1].Position.y = y1;
		m_VertexData.Vertices[iterator_index * 4 + 1].TextureCoordinates.x = u2;
		m_VertexData.Vertices[iterator_index * 4 + 1].TextureCoordinates.y = v1;
		m_VertexData.Vertices[iterator_index * 4 + 2].Position.x = x1;
		m_VertexData.Vertices[iterator_index * 4 + 2].Position.y = y2;
		m_VertexData.Vertices[iterator_index * 4 + 2].TextureCoordinates.x = u1;
		m_VertexData.Vertices[iterator_index * 4 + 2].TextureCoordinates.y = v2;
		m_VertexData.Vertices[iterator_index * 4 + 3].Position.x = x2;
		m_VertexData.Vertices[iterator_index * 4 + 3].Position.y = y2;
		m_VertexData.Vertices[iterator_index * 4 + 3].TextureCoordinates.x = u2;
		m_VertexData.Vertices[iterator_index * 4 + 3].TextureCoordinates.y = v2;

		base_x_position += current_bm_char.XAdvance;
		++iterator_index;
	}

	JWImage::UpdateVertexBuffer();

	JWImage::Draw();
}
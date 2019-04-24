#include "JWInstantText.h"
#include "JWDX.h"

using namespace JWEngine;

void JWInstantText::Create(JWDX& DX, XMFLOAT2 WindowSize, STRING BaseDirectory, STRING FontFileName) noexcept
{
	if (!m_IsCreated)
	{
		m_BaseDirectory = BaseDirectory;

		if (!m_FontParser.IsParsed())
		{
			// Font is not yet parsed, then parse it.
			// The font data(ms_FontData) is static, so we can run this code only once per process,
			// no matter how many JWInstantTexts or windows are made.

			if (!m_FontParser.Parse(StringToWstring(BaseDirectory + FontFileName + ".fnt")))
			{
				JW_ERROR_ABORT("Failed to parse the font file.");
			}
		}

		// Set JWDX pointer.
		m_pDX = &DX;

		// Set orthographic projection matrix.
		m_MatrixProjOrthographic = XMMatrixOrthographicLH(WindowSize.x, WindowSize.y, KOrthographicNearZ, KOrthographicFarZ);

		CreateInstantTextVertexBuffer();
		CreateInstantTextIndexBuffer();

		LoadImageFromFile(BaseDirectory, FontFileName + "_0.png");

		m_IsCreated = true;
	}
}

void JWInstantText::Destroy() noexcept
{
	JW_RELEASE(m_FontTextureSRV);

	JW_RELEASE(m_IndexBuffer);
	JW_RELEASE(m_VertexBuffer);
}

PRIVATE void JWInstantText::CreateInstantTextVertexBuffer() noexcept
{
	m_VertexData.vVertices.clear();
	m_VertexData.vVertices.reserve(KMaxInsantTextLength * 4);

	for (int iterator_count = 0; iterator_count < KMaxInsantTextLength * 4; ++iterator_count)
	{
		m_VertexData.vVertices.push_back(SVertexText());
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

PRIVATE inline void JWInstantText::LoadImageFromFile(STRING Directory, STRING FileName) noexcept
{
	STRING path_string = Directory + FileName;
	WSTRING w_path = StringToWstring(path_string);

	CreateWICTextureFromFile(m_pDX->GetDevice(), w_path.c_str(), nullptr, &m_FontTextureSRV, 0);
}

void JWInstantText::BeginRendering() noexcept
{
	// Set rasterizer state
	if (m_pDX->GetRasterizerState() == ERasterizerState::WireFrame)
	{
		m_ShouldToggleWireFrame = true;
		m_pDX->SwitchRasterizerState();
	}
	
	// Set blend state
	m_pDX->SetBlendState(EBlendState::Transprent);

	// Disable Z-buffer for 2D drawing
	m_pDX->SetDepthStencilState(EDepthStencilState::ZDisabled);

	// Set VS
	m_pDX->SetVS(EVertexShader::VSIntantText);

	// Update VS constant buffer (WVP matrix, which in reality is WO matrix.)
	m_VSCBSpace.WVP = XMMatrixTranspose(m_MatrixProjOrthographic);
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Set PS
	m_pDX->SetPS(EPixelShader::PSIntantText);

	// Set PS texture and sampler
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_FontTextureSRV);
	m_pDX->SetPSSamplerState(ESamplerState::MinMagMipLinearWrap);

	// Empty the vertex data
	m_VertexData.EmptyData();

	// Initialize text length
	m_TotalTextLength = 0;
}

void JWInstantText::RenderText(const WSTRING& Text, XMFLOAT2 Position, XMFLOAT4 FontColorRGB) noexcept
{
	// Throw if total text length is larger than the limit
	uint32_t text_length = static_cast<uint32_t>(Text.length());
	if ((m_TotalTextLength + text_length) < KMaxInsantTextLength)
	{
		float window_width_half = m_pDX->GetWindowSize().x / 2.0f;
		float window_height_half = m_pDX->GetWindowSize().y / 2.0f;
		float texture_width = m_FontParser.GetFontTextureWidth();
		float texture_height = m_FontParser.GetFontTextureHeight();
		float base_x_position = -window_width_half + Position.x;
		float base_y_position = window_height_half - Position.y;
		float x1{}, x2{}, y1{}, y2{};
		float u1{}, u2{}, v1{}, v2{};
		BMFont::BMChar current_bm_char{};

		uint64_t vertex_id_base{ static_cast<uint64_t>(m_TotalTextLength) * 4 };
		uint64_t vertex_id{};

		for (auto iterator_char : Text)
		{
			current_bm_char = m_FontParser.GetBMCharFromWideCharacter(iterator_char);

			u1 = current_bm_char.X_f / texture_width;
			v1 = current_bm_char.Y_f / texture_height;
			u2 = u1 + current_bm_char.Width_f / texture_width;
			v2 = v1 + current_bm_char.Height_f / texture_height;

			x1 = base_x_position + current_bm_char.XOffset_f;
			y1 = base_y_position - current_bm_char.YOffset_f;
			x2 = x1 + current_bm_char.Width_f;
			y2 = y1 - current_bm_char.Height_f;

			m_VertexData.vVertices[vertex_id_base + vertex_id * 4].Position.x = x1;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4].Position.y = y1;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4].TextureCoordinates.x = u1;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4].TextureCoordinates.y = v1;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4].Color = FontColorRGB;

			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 1].Position.x = x2;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 1].Position.y = y1;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 1].TextureCoordinates.x = u2;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 1].TextureCoordinates.y = v1;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 1].Color = FontColorRGB;

			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 2].Position.x = x1;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 2].Position.y = y2;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 2].TextureCoordinates.x = u1;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 2].TextureCoordinates.y = v2;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 2].Color = FontColorRGB;

			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 3].Position.x = x2;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 3].Position.y = y2;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 3].TextureCoordinates.x = u2;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 3].TextureCoordinates.y = v2;
			m_VertexData.vVertices[vertex_id_base + vertex_id * 4 + 3].Color = FontColorRGB;

			base_x_position += current_bm_char.XAdvance;

			++vertex_id;
		}

		// Increase total text length
		m_TotalTextLength += text_length;
	}
}

void JWInstantText::EndRendering() noexcept
{
	// Update vertex buffer
	m_pDX->UpdateDynamicResource(m_VertexBuffer, m_VertexData.GetPtrData(), m_VertexData.GetByteSize());

	// Set IA primitive topology
	m_pDX->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, m_VertexData.GetPtrStride(), m_VertexData.GetPtrOffset());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// @important for performance (Draw ONLY the visible vertices)
	// Draw indexed 
	m_pDX->GetDeviceContext()->DrawIndexed(3 * m_TotalTextLength * 4, 0, 0);

	// Restore rasterizer state
	if (m_ShouldToggleWireFrame)
	{
		m_pDX->SwitchRasterizerState();
		m_ShouldToggleWireFrame = false;
	}	
}
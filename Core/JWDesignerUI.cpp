#include "JWDesignerUI.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWDesignerUI::~JWDesignerUI()
{
	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWDesignerUI::Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	m_BaseDirectory = BaseDirectory;

	MakeGrid(50.0f, 50.0f);

	// Light model
	m_LightModel.Create(DX, Camera);
	LoadLightModel();

	// Mini axis
	m_MiniAxis.Create(DX, Camera);
	MakeMiniAxis();

	m_IsValid = true;
}

PRIVATE void JWDesignerUI::MakeGrid(float XSize, float ZSize, float GridInterval) noexcept
{
	XSize = max(XSize, 0);
	ZSize = max(ZSize, 0);

	int grid_count_x = static_cast<int>(XSize / GridInterval) + 1;
	int grid_count_z = static_cast<int>(ZSize / GridInterval) + 1;

	int total_grid_count{};
	float position_x{}, position_z{};
	for (int i{}; i < grid_count_z; ++i)
	{
		position_z = -ZSize / 2.0f + static_cast<float>(i) * GridInterval;

		if (position_z)
		{
			AddVertex(SVertexColor(-XSize / 2.0f, 0, position_z));
			AddVertex(SVertexColor(+XSize / 2.0f, 0, position_z));

			AddIndex(SIndex2(total_grid_count * 2, total_grid_count * 2 + 1));

			++total_grid_count;
		}
	}
	for (int i{}; i < grid_count_x; ++i)
	{
		position_x = -XSize / 2.0f + static_cast<float>(i) * GridInterval;
		
		if (position_x)
		{
			AddVertex(SVertexColor(position_x, 0, -ZSize / 2.0f));
			AddVertex(SVertexColor(position_x, 0, +ZSize / 2.0f));

			AddIndex(SIndex2(total_grid_count * 2, total_grid_count * 2 + 1));

			++total_grid_count;
		}
	}

	// X Axis
	AddVertex(SVertexColor(XMFLOAT3(-KAxisLength / 2.0f, 0, 0), KXAxisColor));
	AddVertex(SVertexColor(XMFLOAT3(+KAxisLength / 2.0f, 0, 0), KXAxisColor));
	AddIndex(SIndex2(total_grid_count * 2, total_grid_count * 2 + 1));
	++total_grid_count;

	// Y Axis
	AddVertex(SVertexColor(XMFLOAT3(0, -KAxisLength / 2.0f, 0), KYAxisColor));
	AddVertex(SVertexColor(XMFLOAT3(0, +KAxisLength / 2.0f, 0), KYAxisColor));
	AddIndex(SIndex2(total_grid_count * 2, total_grid_count * 2 + 1));
	++total_grid_count;

	// Z Axis
	AddVertex(SVertexColor(XMFLOAT3(0, 0, -KAxisLength / 2.0f), KZAxisColor));
	AddVertex(SVertexColor(XMFLOAT3(0, 0, +KAxisLength / 2.0f), KZAxisColor));
	AddIndex(SIndex2(total_grid_count * 2, total_grid_count * 2 + 1));
	++total_grid_count;

	AddEnd();
}

PRIVATE auto JWDesignerUI::AddVertex(const SVertexColor& Vertex) noexcept->JWDesignerUI&
{
	m_VertexData.Vertices.push_back(Vertex);

	return *this;
}

PRIVATE auto JWDesignerUI::AddIndex(const SIndex2& Index) noexcept->JWDesignerUI&
{
	m_IndexData.Indices.push_back(Index);

	return *this;
}

PRIVATE void JWDesignerUI::AddEnd() noexcept
{
	// Create vertex buffer
	m_pDX->CreateStaticVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);
}

PRIVATE void JWDesignerUI::MakeMiniAxis() noexcept
{
	float window_width = static_cast<float>(m_pDX->GetWindowSize().Width);

	m_MiniAxis.AddLine(SLineData(XMFLOAT2(window_width - 60.0f, 60.0f), XMFLOAT2(20, 0), KXAxisColor));
	m_MiniAxis.AddLine(SLineData(XMFLOAT2(window_width - 60.0f, 60.0f), XMFLOAT2(0, 20), KYAxisColor));
	m_MiniAxis.AddLine(SLineData(XMFLOAT2(window_width - 60.0f, 60.0f), XMFLOAT2(14, 14), KZAxisColor));
	m_MiniAxis.AddEnd();
}

PRIVATE void JWDesignerUI::LoadLightModel() noexcept
{
	JWAssimpLoader assimp_loader;
	m_LightModel.SetModelData(assimp_loader.LoadObj(m_BaseDirectory + KAssetDirectory, KLightModelFileName));
	m_LightModel.SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f));
	m_LightModel.ShouldBeLit(false);
}

PRIVATE void JWDesignerUI::Update() noexcept
{
	// Enable Z-buffer for 3D drawing
	m_pDX->SetDepthStencilState(EDepthStencilState::ZEnabled);

	// Set color VS, PS
	m_pDX->SetColorVS();
	m_pDX->SetColorPS();

	// Set VS constant buffer
	m_ColorVSConstantBufferData.WVP = XMMatrixTranspose(XMMatrixIdentity() * m_pCamera->GetViewProjectionMatrix());
	m_pDX->SetColorVSConstantBufferData(m_ColorVSConstantBufferData);
}

void JWDesignerUI::UpdateLightData(const SLightData& Ambient, const SLightData& Directional, const VECTOR<SLightData>& LightsList) noexcept
{
	m_pAmbientLightData = &Ambient;
	m_pDirectionalLightData = &Directional;
	m_pLightsData = &LightsList;

	m_IsLightDataUpdated = true;
}

void JWDesignerUI::Draw() noexcept
{
	Update();

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, m_VertexData.GetPtrStride(), m_VertexData.GetPtrOffset());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.GetCount(), 0, 0);

	// Draw light models
	DrawLightModel();
}

PRIVATE void JWDesignerUI::DrawLightModel() noexcept
{
	if (m_IsLightDataUpdated)
	{
		if (m_pAmbientLightData->LightType != ELightType::Invalid)
		{
			m_LightModel.SetTranslation(m_pAmbientLightData->Position);
			m_LightModel.Draw();
		}
		if (m_pDirectionalLightData->LightType != ELightType::Invalid)
		{
			m_LightModel.SetTranslation(m_pDirectionalLightData->Position);
			m_LightModel.Draw();
		}
		if (m_pLightsData->size())
		{
			for (const auto& light : *m_pLightsData)
			{
				m_LightModel.SetTranslation(light.Position);
				m_LightModel.Draw();
			}
		}
	}
}

void JWDesignerUI::DrawMiniAxis() noexcept
{
	m_MiniAxis.UpdateLines();
	m_MiniAxis.Draw();
}
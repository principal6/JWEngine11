#include "JWDesignerUI.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWDesignerUI::~JWDesignerUI()
{
	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWDesignerUI::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	m_MatrixWorld = XMMatrixIdentity();

	MakeGrid(50.0f, 50.0f);

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
	AddVertex(SVertexColor(XMFLOAT3(-AxisLength / 2.0f, 0, 0), XAxisColor));
	AddVertex(SVertexColor(XMFLOAT3(+AxisLength / 2.0f, 0, 0), XAxisColor));
	AddIndex(SIndex2(total_grid_count * 2, total_grid_count * 2 + 1));
	++total_grid_count;

	// Y Axis
	AddVertex(SVertexColor(XMFLOAT3(0, -AxisLength / 2.0f, 0), YAxisColor));
	AddVertex(SVertexColor(XMFLOAT3(0, +AxisLength / 2.0f, 0), YAxisColor));
	AddIndex(SIndex2(total_grid_count * 2, total_grid_count * 2 + 1));
	++total_grid_count;

	// Z Axis
	AddVertex(SVertexColor(XMFLOAT3(0, 0, -AxisLength / 2.0f), ZAxisColor));
	AddVertex(SVertexColor(XMFLOAT3(0, 0, +AxisLength / 2.0f), ZAxisColor));
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

PRIVATE void JWDesignerUI::Update() noexcept
{
	// Set VS constant buffer
	m_ColorVSConstantBufferData.WVP = XMMatrixTranspose(m_MatrixWorld * m_pCamera->GetViewProjectionMatrix());
	m_pDX->SetColorVSConstantBufferData(m_ColorVSConstantBufferData);
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
}
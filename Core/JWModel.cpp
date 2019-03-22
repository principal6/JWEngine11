#include "JWModel.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWModel::~JWModel()
{
	JW_RELEASE(m_TextureShaderResourceView);
	
	JW_RELEASE(m_NormalVertexBuffer);
	JW_RELEASE(m_NormalIndexBuffer);

	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWModel::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	m_MatrixWorld = XMMatrixIdentity();
	m_MatrixTranslation = XMMatrixIdentity();
	m_MatrixRotation = XMMatrixIdentity();
	m_MatrixScale = XMMatrixIdentity();

	m_IsValid = true;
}

void JWModel::SetModelData(SModelData ModelData) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsModelLoaded);

	CheckValidity();

	m_VertexData = ModelData.VertexData;
	m_IndexData = ModelData.IndexData;
	AddEnd();

	// Create texture if there is
	if (ModelData.HasTexture)
	{
		m_HasTexture = true;
		CreateTexture(ModelData.TextureFileNameW);
	}

	// For normal line drawing
	size_t iterator_vertex{};
	XMFLOAT3 second_vertex_position{};
	for (const auto& vertex : ModelData.VertexData.Vertices)
	{
		second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
		second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
		second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

		NormalAddVertex(SVertex(vertex.Position, KDefaultColorNormals));
		NormalAddVertex(SVertex(second_vertex_position, KDefaultColorNormals));
		NormalAddIndex(SIndex2(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
		++iterator_vertex;
	}
	NormalAddEnd();

	m_IsModelLoaded = true;
}

void JWModel::SetModel2Data(SModel2Data Model2Data) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsMode2lLoaded);

	CheckValidity();

	m_NormalVertexData = Model2Data.VertexData;
	m_NormalIndexData = Model2Data.IndexData;
	NormalAddEnd();

	m_IsMode2lLoaded = true;
}

PRIVATE void JWModel::CreateTexture(WSTRING TextureFileName) noexcept
{
	CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &m_TextureShaderResourceView, 0);
}

PRIVATE void JWModel::CheckValidity() const noexcept
{
	if (!m_IsValid)
	{
		JWAbort("JWModel object not valid. You must call JWModel::Create() first");
	}
}

PRIVATE auto JWModel::AddVertex(const SVertex& Vertex) noexcept->JWModel&
{
	m_VertexData.Vertices.push_back(Vertex);

	return *this;
}

PRIVATE auto JWModel::AddIndex(const SIndex3& Index) noexcept->JWModel&
{
	m_IndexData.Indices.push_back(Index);

	return *this;
}

PRIVATE void JWModel::AddEnd() noexcept
{
	// Create vertex buffer
	m_pDX->CreateStaticVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);
}

PRIVATE auto JWModel::NormalAddVertex(const SVertex& Vertex) noexcept->JWModel&
{
	m_NormalVertexData.Vertices.push_back(Vertex);

	return *this;
}

PRIVATE auto JWModel::NormalAddIndex(const SIndex2& Index) noexcept->JWModel&
{
	m_NormalIndexData.Indices.push_back(Index);

	return *this;
}

PRIVATE void JWModel::NormalAddEnd() noexcept
{
	// Create vertex buffer
	m_pDX->CreateStaticVertexBuffer(m_NormalVertexData.GetByteSize(), m_NormalVertexData.GetPtrData(), &m_NormalVertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_NormalIndexData.GetByteSize(), m_NormalIndexData.GetPtrData(), &m_NormalIndexBuffer);
}

auto JWModel::SetWorldMatrixToIdentity() noexcept->JWModel&
{
	m_MatrixWorld = XMMatrixIdentity();

	return *this;
}

auto JWModel::SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder Order) noexcept->JWModel&
{
	m_CalculationOrder = Order;

	UpdateWorldMatrix();

	return *this;
}

auto JWModel::SetTranslation(XMFLOAT3 Offset) noexcept->JWModel&
{
	m_MatrixTranslation = XMMatrixTranslation(Offset.x, Offset.y, Offset.z);

	UpdateWorldMatrix();

	return *this;
}

auto JWModel::SetRotation(XMFLOAT4 RotationAxis, float Angle) noexcept->JWModel&
{
	XMVECTOR rotation_axis = XMVectorSet(RotationAxis.x, RotationAxis.y, RotationAxis.z, RotationAxis.w);
	m_MatrixRotation = XMMatrixRotationAxis(rotation_axis, Angle);

	UpdateWorldMatrix();

	return *this;
}

auto JWModel::SetScale(XMFLOAT3 Scale) noexcept->JWModel&
{
	m_MatrixScale = XMMatrixScaling(Scale.x, Scale.y, Scale.z);

	UpdateWorldMatrix();

	return *this;
}

PRIVATE void JWModel::UpdateWorldMatrix() noexcept
{
	switch (m_CalculationOrder)
	{
	case JWEngine::EWorldMatrixCalculationOrder::TransRotScale:
		m_MatrixWorld = m_MatrixTranslation * m_MatrixRotation * m_MatrixScale;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::TransScaleRot:
		m_MatrixWorld = m_MatrixTranslation * m_MatrixScale * m_MatrixRotation;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::RotTransScale:
		m_MatrixWorld = m_MatrixRotation * m_MatrixTranslation * m_MatrixScale;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::RotScaleTrans:
		m_MatrixWorld = m_MatrixRotation * m_MatrixScale * m_MatrixTranslation;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::ScaleTransRot:
		m_MatrixWorld = m_MatrixScale * m_MatrixTranslation * m_MatrixRotation;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::ScaleRotTrans:
		m_MatrixWorld = m_MatrixScale * m_MatrixRotation * m_MatrixTranslation;
		break;
	default:
		break;
	}

	// Update world position of the model
	m_WorldPosition = XMVector3TransformCoord(XMVectorZero(), m_MatrixWorld);
}

auto JWModel::ShouldDrawNormals(bool Value) noexcept->JWModel&
{
	m_ShouldDrawNormals = Value;

	return *this;
}

auto JWModel::ShouldBeLit(bool Value) noexcept->JWModel&
{
	m_ShouldBeLit = Value;

	return *this;
}

auto JWModel::GetWorldPosition() noexcept->XMVECTOR
{
	return m_WorldPosition;
}

auto JWModel::GetDistanceFromCamera() noexcept->float
{
	float distance_x = XMVectorGetX(m_WorldPosition) - XMVectorGetX(m_pCamera->GetPosition());
	float distance_y = XMVectorGetY(m_WorldPosition) - XMVectorGetY(m_pCamera->GetPosition());
	float distance_z = XMVectorGetZ(m_WorldPosition) - XMVectorGetZ(m_pCamera->GetPosition());

	return (distance_x * distance_x + distance_y * distance_y + distance_z * distance_z);
}

PRIVATE void JWModel::Update() noexcept
{
	// Enable Z-buffer for 3D drawing
	m_pDX->SetDepthStencilState(EDepthStencilState::ZEnabled);

	// Set default VS & PS
	m_pDX->SetDefaultVS();
	m_pDX->SetDefaultPS();

	// Set VS constant buffer
	m_DefaultVSCBData.WVP = XMMatrixTranspose(m_MatrixWorld * m_pCamera->GetViewProjectionMatrix());
	m_DefaultVSCBData.World = XMMatrixTranspose(m_MatrixWorld);
	m_pDX->SetDefaultVSCB(m_DefaultVSCBData);

	// Set PS constant buffer
	m_pDX->SetDefaultPSCBDefaultFlags(m_HasTexture, m_ShouldBeLit);

	// Set PS texture and sampler
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->SetPSSamplerState(ESamplerState::LinearWrap);
}

void JWModel::Draw() noexcept
{
	Update();

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, m_VertexData.GetPtrStride(), m_VertexData.GetPtrOffset());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.GetCount(), 0, 0);

	// Draw normals
	if ((m_ShouldDrawNormals) || (m_IsMode2lLoaded))
	{
		DrawNormals();
	}
}

PRIVATE void JWModel::DrawNormals() noexcept
{
	// Set PS constant buffer
	m_pDX->SetDefaultPSCBDefaultFlags(false, false);

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_NormalVertexBuffer, m_NormalVertexData.GetPtrStride(), m_NormalVertexData.GetPtrOffset());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_NormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(m_NormalIndexData.GetCount(), 0, 0);
}
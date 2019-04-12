#include "JWModel.h"
#include "JWDX.h"

using namespace JWEngine;

void JWModel::Create(JWDX& DX, STRING& BaseDirectory) noexcept
{
	// Set JWDX pointer.
	m_pDX = &DX;

	// Set BaseDirectory pointer.
	m_pBaseDirectory = &BaseDirectory;
}

void JWModel::Destroy() noexcept
{
	JW_RELEASE(ModelVertexBuffer);
	JW_RELEASE(ModelIndexBuffer);

	JW_RELEASE(NormalVertexBuffer);
	JW_RELEASE(NormalIndexBuffer);
}

void JWModel::SetNonRiggedModelData(SNonRiggedModelData ModelData) noexcept
{
	if (m_RenderType != ERenderType::Invalid)
	{
		return;
	}

	m_RenderType = ERenderType::Model_Static;

	// Save the model data
	NonRiggedModelData = ModelData;

	CreateModelVertexIndexBuffers();

	// For normal line drawing
	NormalData.VertexData.Clear();
	NormalData.IndexData.Clear();

	size_t iterator_vertex{};
	XMFLOAT3 second_vertex_position{};
	for (const auto& vertex : ModelData.VertexData.vVertices)
	{
		second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
		second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
		second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

		NormalData.VertexData.vVertices.push_back(SVertexNonRiggedModel(vertex.Position, KDefaultColorNormals));
		NormalData.VertexData.vVertices.push_back(SVertexNonRiggedModel(second_vertex_position, KDefaultColorNormals));
		NormalData.IndexData.vIndices.push_back(SIndexLine(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
		++iterator_vertex;
	}
	
	// Create vertex buffer for normals
	m_pDX->CreateStaticVertexBuffer(NormalData.VertexData.GetByteSize(), NormalData.VertexData.GetPtrData(), &NormalVertexBuffer);

	// Create index buffer for normals
	m_pDX->CreateIndexBuffer(NormalData.IndexData.GetByteSize(), NormalData.IndexData.GetPtrData(), &NormalIndexBuffer);
}

void JWModel::SetDynamicModelData(SNonRiggedModelData ModelData) noexcept
{
	if (m_RenderType != ERenderType::Invalid)
	{
		return;
	}

	m_RenderType = ERenderType::Model_Dynamic;

	// Save the model data
	NonRiggedModelData = ModelData;

	CreateModelVertexIndexBuffers();

	// For normal line drawing
	NormalData.VertexData.Clear();
	NormalData.IndexData.Clear();

	size_t iterator_vertex{};
	XMFLOAT3 second_vertex_position{};
	for (const auto& vertex : ModelData.VertexData.vVertices)
	{
		second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
		second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
		second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

		NormalData.VertexData.vVertices.push_back(SVertexNonRiggedModel(vertex.Position, KDefaultColorNormals));
		NormalData.VertexData.vVertices.push_back(SVertexNonRiggedModel(second_vertex_position, KDefaultColorNormals));
		NormalData.IndexData.vIndices.push_back(SIndexLine(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
		++iterator_vertex;
	}

	// Create vertex buffer for normals
	m_pDX->CreateStaticVertexBuffer(NormalData.VertexData.GetByteSize(), NormalData.VertexData.GetPtrData(), &NormalVertexBuffer);

	// Create index buffer for normals
	m_pDX->CreateIndexBuffer(NormalData.IndexData.GetByteSize(), NormalData.IndexData.GetPtrData(), &NormalIndexBuffer);
}

void JWModel::SetRiggedModelData(SRiggedModelData ModelData) noexcept
{
	if (m_RenderType != ERenderType::Invalid)
	{
		return;
	}

	m_RenderType = ERenderType::Model_Rigged;

	// Save the model data
	RiggedModelData = ModelData;

	CreateModelVertexIndexBuffers();

	// For normal line drawing
	NormalData.VertexData.Clear();
	NormalData.IndexData.Clear();

	size_t iterator_vertex{};
	XMFLOAT3 second_vertex_position{};
	for (const auto& vertex : ModelData.VertexData.vVertices)
	{
		second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
		second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
		second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

		NormalData.VertexData.vVertices.push_back(SVertexNonRiggedModel(vertex.Position, KDefaultColorNormals));
		NormalData.VertexData.vVertices.push_back(SVertexNonRiggedModel(second_vertex_position, KDefaultColorNormals));
		NormalData.IndexData.vIndices.push_back(SIndexLine(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
		++iterator_vertex;
	}

	// Create vertex buffer for normals
	m_pDX->CreateStaticVertexBuffer(NormalData.VertexData.GetByteSize(), NormalData.VertexData.GetPtrData(), &NormalVertexBuffer);

	// Create index buffer for normals
	m_pDX->CreateIndexBuffer(NormalData.IndexData.GetByteSize(), NormalData.IndexData.GetPtrData(), &NormalIndexBuffer);
}

PRIVATE void JWModel::CreateModelVertexIndexBuffers() noexcept
{
	switch (m_RenderType)
	{
	case JWEngine::ERenderType::Model_Static:
		// Create vertex buffer
		m_pDX->CreateStaticVertexBuffer(NonRiggedModelData.VertexData.GetByteSize(), NonRiggedModelData.VertexData.GetPtrData(), &ModelVertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(NonRiggedModelData.IndexData.GetByteSize(), NonRiggedModelData.IndexData.GetPtrData(), &ModelIndexBuffer);

		break;
	case JWEngine::ERenderType::Model_Dynamic:
		// Create vertex buffer
		m_pDX->CreateDynamicVertexBuffer(NonRiggedModelData.VertexData.GetByteSize(), NonRiggedModelData.VertexData.GetPtrData(), &ModelVertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(NonRiggedModelData.IndexData.GetByteSize(), NonRiggedModelData.IndexData.GetPtrData(), &ModelIndexBuffer);

		break;
	case JWEngine::ERenderType::Model_Rigged:
		// Create vertex buffer
		m_pDX->CreateStaticVertexBuffer(RiggedModelData.VertexData.GetByteSize(), RiggedModelData.VertexData.GetPtrData(), &ModelVertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(RiggedModelData.IndexData.GetByteSize(), RiggedModelData.IndexData.GetPtrData(), &ModelIndexBuffer);

		break;
	default:
		break;
	}
}

auto JWModel::SetVertex(uint32_t VertexIndex, XMFLOAT3 Position, XMFLOAT4 Color) noexcept->JWModel*
{
	if (m_RenderType == ERenderType::Model_Dynamic)
	{
		if (VertexIndex > NonRiggedModelData.VertexData.GetCount())
		{
			return this;
		}

		NonRiggedModelData.VertexData.vVertices[VertexIndex].Position = Position;
		NonRiggedModelData.VertexData.vVertices[VertexIndex].ColorDiffuse = Color;
	}

	return this;
}

void JWModel::UpdateModel() noexcept
{
	if (m_RenderType == ERenderType::Model_Dynamic)
	{
		D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
		if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(ModelVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
		{
			memcpy(mapped_subresource.pData, NonRiggedModelData.VertexData.GetPtrData(), NonRiggedModelData.VertexData.GetByteSize());

			m_pDX->GetDeviceContext()->Unmap(ModelVertexBuffer, 0);
		}
	}
}
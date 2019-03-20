#include "JWModel.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWModel::~JWModel()
{
	JW_RELEASE(m_TextureSamplerState);
	JW_RELEASE(m_TextureShaderResourceView);
	
	JW_RELEASE(m_NormalVertexBuffer);
	JW_RELEASE(m_NormalIndexBuffer);

	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWModel::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	AVOID_DUPLICATE_CREATION(m_IsValid);

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

void JWModel::LoadModelObj(STRING Directory, STRING FileName) noexcept
{
	CheckValidity();

	Assimp::Importer m_AssimpImporter{};

	const aiScene* m_AssimpScene{ m_AssimpImporter.ReadFile(Directory + FileName, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded) };

	if (m_AssimpScene)
	{
		if (m_AssimpScene->HasMaterials())
		{
			auto material_count{ m_AssimpScene->mNumMaterials };
			auto properties_count{ m_AssimpScene->mMaterials[0]->mNumProperties };
			
			aiString path{};
			m_AssimpScene->mMaterials[material_count - 1]->GetTexture(aiTextureType::aiTextureType_AMBIENT, 0, &path);
			STRING path_string{ Directory + path.C_Str() };

			if (path.length == 0)
			{
				// No texture
				m_HasTexture = FALSE;
			}
			else
			{
				// It has texture
				CreateTexture(StringToWstring(path_string));
				m_HasTexture = TRUE;
			}
		}

		if (m_AssimpScene->HasMeshes())
		{
			auto mesh_count{ m_AssimpScene->mNumMeshes };
			size_t mesh_index{};

			if (m_AssimpScene->mMeshes[mesh_index]->HasPositions())
			{
				size_t vertices_count{ m_AssimpScene->mMeshes[mesh_index]->mNumVertices };

				for (size_t iterator_vertices{}; iterator_vertices < vertices_count; ++iterator_vertices)
				{
					auto& vertex = m_AssimpScene->mMeshes[mesh_index]->mVertices[iterator_vertices];
					auto& texcoord = m_AssimpScene->mMeshes[mesh_index]->mTextureCoords[0][iterator_vertices];
					auto& normal = m_AssimpScene->mMeshes[mesh_index]->mNormals[iterator_vertices];

					AddVertex(SVertex(vertex.x, vertex.y, vertex.z, texcoord.x, texcoord.y, normal.x, normal.y, normal.z));

					// For normal line drawing
					NormalAddVertex(SVertex(vertex.x, vertex.y, vertex.z));
					NormalAddVertex(SVertex(vertex.x + normal.x, vertex.y + normal.y, vertex.z + normal.z));
					NormalAddIndex(SIndex2(static_cast<UINT>(iterator_vertices * 2), static_cast<UINT>(iterator_vertices * 2 + 1)));
				}

				// For normal line drawing
				NormalAddEnd();
			}
			else
			{
				JWAbort("Loaded model doesn't have positions");
			}

			if (m_AssimpScene->mMeshes[mesh_index]->HasFaces())
			{
				size_t faces_count{ m_AssimpScene->mMeshes[mesh_index]->mNumFaces };

				for (size_t iterator_faces{}; iterator_faces < faces_count; ++iterator_faces)
				{
					auto& indices_count = m_AssimpScene->mMeshes[mesh_index]->mFaces[iterator_faces].mNumIndices;
					auto& indices = m_AssimpScene->mMeshes[mesh_index]->mFaces[iterator_faces].mIndices;

					if (indices_count == 3)
					{
						AddIndex(SIndex(indices[0], indices[1], indices[2]));
					}
					else
					{
						JWAbort("Index count is not 3");
					}
				}
			}
			else
			{
				JWAbort("Loaded model doesn't have faces");
			}
		}
	}
	else
	{
		JWAbort("Model file not loaded.");
	}

	AddEnd();
}

PRIVATE void JWModel::CreateTexture(WSTRING TextureFileName) noexcept
{
	CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &m_TextureShaderResourceView, 0);

	CreateSamplerState();
}

PRIVATE void JWModel::CreateSamplerState() noexcept
{
	AVOID_DUPLICATE_CREATION(m_TextureSamplerState);

	D3D11_SAMPLER_DESC sampler_description{};
	sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_description.MinLOD = 0;
	sampler_description.MaxLOD = D3D11_FLOAT32_MAX;

	m_pDX->GetDevice()->CreateSamplerState(&sampler_description, &m_TextureSamplerState);
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

PRIVATE auto JWModel::AddIndex(const SIndex& Index) noexcept->JWModel&
{
	m_IndexData.Indices.push_back(Index);

	return *this;
}

PRIVATE void JWModel::AddEnd() noexcept
{
	CheckValidity();

	// Calculate the count of vertices
	m_VertexData.Count = static_cast<UINT>(m_VertexData.Vertices.size());

	// Calculate the count of indices
	m_IndexData.Count = static_cast<UINT>(m_IndexData.Indices.size() * 3);

	// Create vertex buffer
	CreateVertexBuffer();

	// Create index buffer
	CreateIndexBuffer();
}

PRIVATE void JWModel::CreateVertexBuffer() noexcept
{
	D3D11_BUFFER_DESC vertex_buffer_description{};
	vertex_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_description.ByteWidth = sizeof(SVertex) * m_VertexData.Count;
	vertex_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_description.CPUAccessFlags = 0;
	vertex_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_buffer_data{};
	vertex_buffer_data.pSysMem = &m_VertexData.Vertices[0];

	// Create vertex buffer
	m_pDX->GetDevice()->CreateBuffer(&vertex_buffer_description, &vertex_buffer_data, &m_VertexBuffer);
}

PRIVATE void JWModel::CreateIndexBuffer() noexcept
{
	D3D11_BUFFER_DESC index_buffer_description{};
	index_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_description.ByteWidth = sizeof(DWORD) * m_IndexData.Count;
	index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_description.CPUAccessFlags = 0;
	index_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA index_buffer_data{};
	index_buffer_data.pSysMem = &m_IndexData.Indices[0];

	// Create index buffer
	m_pDX->GetDevice()->CreateBuffer(&index_buffer_description, &index_buffer_data, &m_IndexBuffer);
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
	CheckValidity();

	// Calculate the count of vertices
	m_NormalVertexData.Count = static_cast<UINT>(m_NormalVertexData.Vertices.size());

	// Calculate the count of indices
	m_NormalIndexData.Count = static_cast<UINT>(m_NormalIndexData.Indices.size() * 2);

	// Create vertex buffer
	NormalCreateVertexBuffer();

	// Create index buffer
	NormalCreateIndexBuffer();
}

PRIVATE void JWModel::NormalCreateVertexBuffer() noexcept
{
	D3D11_BUFFER_DESC vertex_buffer_description{};
	vertex_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_description.ByteWidth = sizeof(SVertex) * m_NormalVertexData.Count;
	vertex_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_description.CPUAccessFlags = 0;
	vertex_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_buffer_data{};
	vertex_buffer_data.pSysMem = &m_NormalVertexData.Vertices[0];

	// Create vertex buffer
	m_pDX->GetDevice()->CreateBuffer(&vertex_buffer_description, &vertex_buffer_data, &m_NormalVertexBuffer);
}

PRIVATE void JWModel::NormalCreateIndexBuffer() noexcept
{
	D3D11_BUFFER_DESC index_buffer_description{};
	index_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_description.ByteWidth = sizeof(DWORD) * m_NormalIndexData.Count;
	index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_description.CPUAccessFlags = 0;
	index_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA index_buffer_data{};
	index_buffer_data.pSysMem = &m_NormalIndexData.Indices[0];

	// Create index buffer
	m_pDX->GetDevice()->CreateBuffer(&index_buffer_description, &index_buffer_data, &m_NormalIndexBuffer);
}

void JWModel::SetWorldMatrixToIdentity() noexcept
{
	m_MatrixWorld = XMMatrixIdentity();
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

void JWModel::ShouldDrawNormals(bool Value) noexcept
{
	m_ShouldDrawNormals = Value;
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

PRIVATE void JWModel::UpdateModel() noexcept
{
	// Set VS constant buffer
	m_DefaultVSConstantBufferData.WVP = XMMatrixTranspose(m_MatrixWorld * m_pCamera->GetViewProjectionMatrix());
	m_DefaultVSConstantBufferData.World = XMMatrixTranspose(m_MatrixWorld);
	m_pDX->SetDefaultVSConstantBufferData(m_DefaultVSConstantBufferData);

	// Set PS constant buffer
	m_DefaultPSConstantBufferData.HasTexture = m_HasTexture;
	m_DefaultPSConstantBufferData.ColorRGB = DefaultColorNoTexture;
	m_pDX->SetDefaultPSConstantBufferData(m_DefaultPSConstantBufferData);

	// Set PS texture and sampler
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->GetDeviceContext()->PSSetSamplers(0, 1, &m_TextureSamplerState);
}

void JWModel::Draw() noexcept
{
	UpdateModel();

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set IA vertex buffer
	UINT vertex_stride{ sizeof(SVertex) };
	UINT vertex_offset{};
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &vertex_stride, &vertex_offset);

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.Count, 0, 0);

	if (m_ShouldDrawNormals)
	{
		DrawNormals();
	}
}

PRIVATE void JWModel::UpdateNormals() noexcept
{
	// Set VS constant buffer
	m_DefaultVSConstantBufferData.WVP = XMMatrixTranspose(m_MatrixWorld * m_pCamera->GetViewProjectionMatrix());
	m_DefaultVSConstantBufferData.World = XMMatrixTranspose(m_MatrixWorld);
	m_pDX->SetDefaultVSConstantBufferData(m_DefaultVSConstantBufferData);

	// Set PS constant buffer
	m_DefaultPSConstantBufferData.HasTexture = FALSE;
	m_DefaultPSConstantBufferData.ColorRGB = DefaultColorNormals;
	m_pDX->SetDefaultPSConstantBufferData(m_DefaultPSConstantBufferData);

	// Set PS texture and sampler
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->GetDeviceContext()->PSSetSamplers(0, 1, &m_TextureSamplerState);
}

PRIVATE void JWModel::DrawNormals() noexcept
{
	UpdateNormals();

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set IA vertex buffer
	UINT vertex_stride{ sizeof(SVertex) };
	UINT vertex_offset{};
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_NormalVertexBuffer, &vertex_stride, &vertex_offset);

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_NormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(m_NormalIndexData.Count, 0, 0);
}
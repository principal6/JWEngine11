#include "JWModel.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWModel::~JWModel()
{
	m_TextureSamplerState->Release();
	m_TextureShaderResourceView->Release();

	m_VertexBuffer->Release();
	m_IndexBuffer->Release();
}

void JWModel::Create(JWDX& DX, JWCamera& Camera) noexcept
{
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

			CreateTexture(StringToWstring(path_string));
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

					AddVertex(SVertex(vertex.x, vertex.y, vertex.z, texcoord.x, texcoord.y));
				}
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
	if (m_TextureSamplerState)
	{
		// To avoid duplicate creation.
		return;
	}

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

	// Set the primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
	// Set WVP matrix and send it to the constant buffer for vertex shader
	m_WVP = m_MatrixWorld * m_pCamera->GetViewProjectionMatrix();
	m_pDX->SetConstantBufferData(SConstantBufferDataPerObject(XMMatrixTranspose(m_WVP)));

	// Set texture and sampler for pixel shader
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->GetDeviceContext()->PSSetSamplers(0, 1, &m_TextureSamplerState);
}

void JWModel::Draw() noexcept
{
	Update();

	// Set vertex buffer
	UINT vertex_stride{ sizeof(SVertex) };
	UINT vertex_offset{};
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &vertex_stride, &vertex_offset);

	// Set index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	m_pDX->GetDeviceContext()->DrawIndexed(m_IndexData.Count, 0, 0);
}
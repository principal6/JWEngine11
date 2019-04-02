#include "JWModel.h"
#include "JWDX.h"

using namespace JWEngine;

JWModel::~JWModel()
{
	JW_RELEASE(TextureShaderResourceView);
	
	JW_RELEASE(NormalVertexBuffer);
	JW_RELEASE(NormalIndexBuffer);

	JW_RELEASE(ModelVertexBuffer);
	JW_RELEASE(ModelIndexBuffer);
}

void JWModel::Create(JWDX& DX) noexcept
{
	// Set JWDX pointer.
	m_pDX = &DX;
}

void JWModel::SetStaticModelData(SStaticModelData ModelData) noexcept
{
	if (m_ModelType != EModelType::Invalid)
	{
		return;
	}

	m_ModelType = EModelType::StaticModel;

	// Save the model data
	StaticModelData = ModelData;

	CreateModelVertexIndexBuffers();

	// Create texture if there is
	if (ModelData.HasTexture)
	{
		CreateTexture(ModelData.TextureFileNameW);
	}

	// For normal line drawing
	NormalData.VertexData.Clear();
	NormalData.IndexData.Clear();

	size_t iterator_vertex{};
	XMFLOAT3 second_vertex_position{};
	for (const auto& vertex : ModelData.VertexData.Vertices)
	{
		second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
		second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
		second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

		NormalData.VertexData.Vertices.push_back(SStaticModelVertex(vertex.Position, KDefaultColorNormals));
		NormalData.VertexData.Vertices.push_back(SStaticModelVertex(second_vertex_position, KDefaultColorNormals));
		NormalData.IndexData.Indices.push_back(SIndex2(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
		++iterator_vertex;
	}
	
	// Create vertex buffer for normals
	m_pDX->CreateStaticVertexBuffer(NormalData.VertexData.GetByteSize(), NormalData.VertexData.GetPtrData(), &NormalVertexBuffer);

	// Create index buffer for normals
	m_pDX->CreateIndexBuffer(NormalData.IndexData.GetByteSize(), NormalData.IndexData.GetPtrData(), &NormalIndexBuffer);
}

void JWModel::SetRiggedModelData(SRiggedModelData ModelData) noexcept
{
	if (m_ModelType != EModelType::Invalid)
	{
		return;
	}

	m_ModelType = EModelType::RiggedModel;

	// Save the model data
	RiggedModelData = ModelData;

	CreateModelVertexIndexBuffers();

	// Create texture if there is
	if (ModelData.HasTexture)
	{
		CreateTexture(ModelData.TextureFileNameW);
	}

	// For normal line drawing
	NormalData.VertexData.Clear();
	NormalData.IndexData.Clear();

	size_t iterator_vertex{};
	XMFLOAT3 second_vertex_position{};
	for (const auto& vertex : ModelData.VertexData.Vertices)
	{
		second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
		second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
		second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

		NormalData.VertexData.Vertices.push_back(SStaticModelVertex(vertex.Position, KDefaultColorNormals));
		NormalData.VertexData.Vertices.push_back(SStaticModelVertex(second_vertex_position, KDefaultColorNormals));
		NormalData.IndexData.Indices.push_back(SIndex2(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
		++iterator_vertex;
	}

	// Create vertex buffer for normals
	m_pDX->CreateStaticVertexBuffer(NormalData.VertexData.GetByteSize(), NormalData.VertexData.GetPtrData(), &NormalVertexBuffer);

	// Create index buffer for normals
	m_pDX->CreateIndexBuffer(NormalData.IndexData.GetByteSize(), NormalData.IndexData.GetPtrData(), &NormalIndexBuffer);
}

void JWModel::SetLineModelData(SLineModelData Model2Data) noexcept
{
	if (m_ModelType != EModelType::Invalid)
	{
		return;
	}

	m_ModelType = EModelType::LineModel;

	NormalData = Model2Data;
	
	// Create vertex buffer for normals
	m_pDX->CreateStaticVertexBuffer(NormalData.VertexData.GetByteSize(), NormalData.VertexData.GetPtrData(), &NormalVertexBuffer);

	// Create index buffer for normals
	m_pDX->CreateIndexBuffer(NormalData.IndexData.GetByteSize(), NormalData.IndexData.GetPtrData(), &NormalIndexBuffer);
}

PRIVATE void JWModel::CreateTexture(WSTRING TextureFileName) noexcept
{
	if (TextureFileName.find(L".dds") != std::string::npos)
	{
		CreateDDSTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &TextureShaderResourceView, 0);
	}
	else
	{
		CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &TextureShaderResourceView, 0);
	}
}

PRIVATE void JWModel::CreateModelVertexIndexBuffers() noexcept
{
	switch (m_ModelType)
	{
	case JWEngine::EModelType::StaticModel:
		// Create vertex buffer
		m_pDX->CreateStaticVertexBuffer(StaticModelData.VertexData.GetByteSize(), StaticModelData.VertexData.GetPtrData(), &ModelVertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(StaticModelData.IndexData.GetByteSize(), StaticModelData.IndexData.GetPtrData(), &ModelIndexBuffer);

		break;
	case JWEngine::EModelType::RiggedModel:
		// Create vertex buffer
		m_pDX->CreateStaticVertexBuffer(RiggedModelData.VertexData.GetByteSize(), RiggedModelData.VertexData.GetPtrData(), &ModelVertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(RiggedModelData.IndexData.GetByteSize(), RiggedModelData.IndexData.GetPtrData(), &ModelIndexBuffer);

		break;
	default:
		break;
	}
}

auto JWModel::AddAnimationFromFile(STRING Directory, STRING ModelFileName) noexcept->JWModel&
{
	if (m_ModelType == EModelType::RiggedModel)
	{
		JWAssimpLoader loader;
		loader.LoadAdditionalAnimationIntoRiggedModel(RiggedModelData, Directory, ModelFileName);
	}

	return *this;
}

auto JWModel::SetAnimation(size_t AnimationID, bool ShouldRepeat) noexcept->JWModel&
{
	if (m_ModelType == EModelType::RiggedModel)
	{
		if (RiggedModelData.AnimationSet.vAnimations.size())
		{
			AnimationID = min(AnimationID, RiggedModelData.AnimationSet.vAnimations.size() - 1);

			// Set animation only when animation id has changed from the previous one.
			if (RiggedModelData.CurrentAnimationID != AnimationID)
			{
				RiggedModelData.CurrentAnimationID = AnimationID;
				RiggedModelData.CurrentAnimationTick = 0;
				RiggedModelData.ShouldRepeatCurrentAnimation = ShouldRepeat;
			}
		}
		else
		{
			// This is rigged model, but it has no animation set
			JWAbort("This model doesn't have any animation set.");
		}
	}

	return *this;
}

auto JWModel::SetPrevAnimation(bool ShouldRepeat) noexcept->JWModel&
{
	if (m_ModelType == EModelType::RiggedModel)
	{
		size_t AnimationID = RiggedModelData.CurrentAnimationID - 1;
		AnimationID = min(AnimationID, RiggedModelData.AnimationSet.vAnimations.size() - 1);

		SetAnimation(AnimationID, ShouldRepeat);
	}

	return *this;
}

auto JWModel::SetNextAnimation(bool ShouldRepeat) noexcept->JWModel&
{
	if (m_ModelType == EModelType::RiggedModel)
	{
		size_t AnimationID = RiggedModelData.CurrentAnimationID + 1;
		if (AnimationID >= RiggedModelData.AnimationSet.vAnimations.size())
		{
			AnimationID = 0;
		}

		SetAnimation(AnimationID, ShouldRepeat);
	}

	return *this;
}

auto JWModel::ToggleTPose() noexcept->JWModel&
{
	if (m_ModelType == EModelType::RiggedModel)
	{
		FlagRenderOption = FlagRenderOption ^ JWFlagRenderOption_DrawTPose;
	}

	return *this;
}
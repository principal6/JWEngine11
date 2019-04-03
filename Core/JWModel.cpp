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

void JWModel::MakeCube(float Size) noexcept
{
	m_ModelType = EModelType::StaticModel;

	float half_size = Size / 2.0f;

	XMFLOAT3 UpColor[4]
		= { {1, 1, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	XMFLOAT3 DownColor[4]
		= { {1, 1, 0}, {0, 1, 1}, {1, 0, 1}, {0, 0, 0} };

	/*
	** Vertex
	*/
	SStaticModelVertexData vert_data;
	// Up (LeftUp - RightUp - LeftDown - RightDown order)
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, +half_size, +half_size, 0, 0, UpColor[0].x, UpColor[0].y, UpColor[0].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, +half_size, +half_size, 1, 0, UpColor[1].x, UpColor[1].y, UpColor[1].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, +half_size, -half_size, 0, 1, UpColor[2].x, UpColor[2].y, UpColor[2].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, +half_size, -half_size, 1, 1, UpColor[3].x, UpColor[3].y, UpColor[3].z, 1));

	// Down
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, -half_size, +half_size, 0, 0, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, -half_size, +half_size, 1, 0, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, -half_size, -half_size, 0, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, -half_size, -half_size, 1, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));

	// Front
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, +half_size, -half_size, 0, 0, UpColor[2].x, UpColor[2].y, UpColor[2].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, +half_size, -half_size, 1, 0, UpColor[3].x, UpColor[3].y, UpColor[3].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, -half_size, -half_size, 0, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, -half_size, -half_size, 1, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));

	// Right
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, +half_size, -half_size, UpColor[3].x, UpColor[3].y, UpColor[3].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, +half_size, +half_size, UpColor[1].x, UpColor[1].y, UpColor[1].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, -half_size, -half_size, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, -half_size, +half_size, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));

	// Back
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, +half_size, +half_size, UpColor[1].x, UpColor[1].y, UpColor[1].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, +half_size, +half_size, UpColor[0].x, UpColor[0].y, UpColor[0].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, -half_size, +half_size, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, -half_size, +half_size, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));

	// Left
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, +half_size, +half_size, UpColor[0].x, UpColor[0].y, UpColor[0].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, +half_size, -half_size, UpColor[2].x, UpColor[2].y, UpColor[2].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, -half_size, +half_size, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, -half_size, -half_size, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));

	/*
	** Index
	*/
	SModelIndexData ind_data;
	for (unsigned int i = 0; i < vert_data.GetCount() / 4; ++i)
	{
		// Clock-wise winding
		ind_data.Indices.push_back(SIndex3( i * 4		, i * 4 + 1	, i * 4 + 2 ));
		ind_data.Indices.push_back(SIndex3( i * 4 + 1	, i * 4 + 3	, i * 4 + 2 ));
	}

	/*
	** Saving model data
	*/
	StaticModelData.HasTexture = false;
	StaticModelData.VertexData = vert_data;
	StaticModelData.IndexData = ind_data;

	// Create buffers
	CreateModelVertexIndexBuffers();
}

void JWModel::MakePyramid(float Height, float Width) noexcept
{
	m_ModelType = EModelType::StaticModel;

	float half_width = Width / 2.0f;

	XMFLOAT3 UpColor{ 1, 1, 1 };
	XMFLOAT3 DownColor[4]
		= { {1, 1, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	/*
	** Vertex
	*/
	SStaticModelVertexData vert_data;
	// Down (LeftUp - RightUp - LeftDown - RightDown order)
	vert_data.Vertices.push_back(SStaticModelVertex(-half_width, 0, +half_width, 0, 0, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_width, 0, +half_width, 1, 0, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_width, 0, -half_width, 0, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_width, 0, -half_width, 1, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));

	// Front (Tip - LeftDown - RightDown order)
	vert_data.Vertices.push_back(SStaticModelVertex(0, Height, 0, 0.5f, 1, UpColor.x, UpColor.y, UpColor.z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_width, 0, -half_width, 0, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_width, 0, -half_width, 1, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));

	// Right
	vert_data.Vertices.push_back(SStaticModelVertex(0, Height, 0, 0.5f, 1, UpColor.x, UpColor.y, UpColor.z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_width, 0, -half_width, 0, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_width, 0, +half_width, 1, 1, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));

	// Back
	vert_data.Vertices.push_back(SStaticModelVertex(0, Height, 0, 0.5f, 1, UpColor.x, UpColor.y, UpColor.z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_width, 0, +half_width, 0, 1, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_width, 0, +half_width, 1, 1, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));

	// Left
	vert_data.Vertices.push_back(SStaticModelVertex(0, Height, 0, 0.5f, 1, UpColor.x, UpColor.y, UpColor.z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_width, 0, +half_width, 0, 1, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_width, 0, -half_width, 1, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));

	/*
	** Index
	*/
	SModelIndexData ind_data;
	ind_data.Indices.push_back(SIndex3(0, 1, 2));
	ind_data.Indices.push_back(SIndex3(1, 3, 2));

	int ind_offset = 4;
	for (unsigned int i = 0; i < (vert_data.GetCount() - 4) / 3; ++i)
	{
		// Clock-wise winding
		ind_data.Indices.push_back(SIndex3(ind_offset + i * 3, ind_offset + i * 3 + 2, ind_offset + i * 3 + 1));
	}

	/*
	** Saving model data
	*/
	StaticModelData.HasTexture = false;
	StaticModelData.VertexData = vert_data;
	StaticModelData.IndexData = ind_data;

	// Create buffers
	CreateModelVertexIndexBuffers();
}

void JWModel::MakeCone(float Height, float Radius, uint8_t Detail) noexcept
{
	m_ModelType = EModelType::StaticModel;

	XMFLOAT3 UpColor{ 0, 0, 1 };
	XMFLOAT3 DownColor{ 0, 1, 0 };

	Detail = max(Detail, 5);
	
	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	SStaticModelVertexData vert_data;
	float stride = XM_2PI / Detail;
	for (uint8_t i = 0; i < Detail; ++i)
	{
		// Down
		vert_data.Vertices.push_back(SStaticModelVertex(0, 0, 0, 0, 0, DownColor.x, DownColor.y, DownColor.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));

		// Side
		vert_data.Vertices.push_back(SStaticModelVertex(0, Height, 0, 0, 0, UpColor.x, UpColor.y, UpColor.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));
	}

	/*
	** Index
	*/
	SModelIndexData ind_data;
	for (unsigned int i = 0; i < vert_data.GetCount() / 3; ++i)
	{
		// Clock-wise winding
		ind_data.Indices.push_back(SIndex3(i * 3, i * 3 + 2, i * 3 + 1));
	}

	/*
	** Saving model data
	*/
	StaticModelData.HasTexture = false;
	StaticModelData.VertexData = vert_data;
	StaticModelData.IndexData = ind_data;

	// Create buffers
	CreateModelVertexIndexBuffers();
}

void JWModel::MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept
{
	m_ModelType = EModelType::StaticModel;

	XMFLOAT3 UpColor{ 0, 0, 1 };
	XMFLOAT3 DownColor{ 0, 1, 0 };

	Detail = max(Detail, 5);

	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	SStaticModelVertexData vert_data;
	float stride = XM_2PI / Detail;
	for (uint8_t i = 0; i < Detail; ++i)
	{
		// Up
		vert_data.Vertices.push_back(SStaticModelVertex(0, Height, 0, 0, 0, UpColor.x, UpColor.y, UpColor.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * i), Height, Radius * sinf(stride * i), 0, 0,
			UpColor.x, UpColor.y, UpColor.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * (i + 1)), Height, Radius * sinf(stride * (i + 1)), 0, 0,
			UpColor.x, UpColor.y, UpColor.z, 1));

		// Down
		vert_data.Vertices.push_back(SStaticModelVertex(0, 0, 0, 0, 0, DownColor.x, DownColor.y, DownColor.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));

		// Side #1 (0 - 1 - 2)
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * i), Height, Radius * sinf(stride * i), 0, 0,
			UpColor.x * 0.9f, UpColor.y * 0.9f, UpColor.z * 0.9f, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * (i + 1)), Height, Radius * sinf(stride * (i + 1)), 0, 0,
			UpColor.x * 0.9f, UpColor.y * 0.9f, UpColor.z * 0.9f, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x * 0.9f, DownColor.y * 0.9f, DownColor.z * 0.9f, 1));

		// Side #2 (1 - 3 - 2)
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * (i + 1)), Height, Radius * sinf(stride * (i + 1)), 0, 0,
			UpColor.x * 0.9f, UpColor.y * 0.9f, UpColor.z * 0.9f, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			DownColor.x * 0.9f, DownColor.y * 0.9f, DownColor.z * 0.9f, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x * 0.9f, DownColor.y * 0.9f, DownColor.z * 0.9f, 1));
	}

	/*
	** Index
	*/
	SModelIndexData ind_data;
	for (unsigned int i = 0; i < vert_data.GetCount() / 3; ++i)
	{
		// Clock-wise winding
		ind_data.Indices.push_back(SIndex3(i * 3, i * 3 + 2, i * 3 + 1));
	}

	/*
	** Saving model data
	*/
	StaticModelData.HasTexture = false;
	StaticModelData.VertexData = vert_data;
	StaticModelData.IndexData = ind_data;

	// Create buffers
	CreateModelVertexIndexBuffers();
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
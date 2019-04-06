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

void JWModel::MakeSquare(float Size, XMFLOAT2 UVMap) noexcept
{
	m_RenderType = ERenderType::Model_Static;

	float half_size = Size / 2.0f;

	XMFLOAT3 Color[4] = { {1, 1, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	/*
	** Vertex
	*/
	SStaticModelVertexData vert_data;
	// (LeftUp - RightUp - LeftDown - RightDown order)
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, 0, +half_size, 0, 0, Color[0].x, Color[0].y, Color[0].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, 0, +half_size, UVMap.x, 0, Color[1].x, Color[1].y, Color[1].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(-half_size, 0, -half_size, 0, UVMap.y, Color[2].x, Color[2].y, Color[2].z, 1));
	vert_data.Vertices.push_back(SStaticModelVertex(+half_size, 0, -half_size, UVMap.x, UVMap.y, Color[3].x, Color[3].y, Color[3].z, 1));

	/*
	** Index
	*/
	SModelIndexData ind_data;
	for (unsigned int i = 0; i < vert_data.GetCount() / 4; ++i)
	{
		// Clock-wise winding
		ind_data.Indices.push_back(SIndex3(i * 4, i * 4 + 1, i * 4 + 2));
		ind_data.Indices.push_back(SIndex3(i * 4 + 1, i * 4 + 3, i * 4 + 2));
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

void JWModel::MakeCircle(float Radius, uint8_t Detail) noexcept
{
	m_RenderType = ERenderType::Model_Static;

	XMFLOAT3 Color{ 0, 0, 1 };

	Detail = max(Detail, 5);

	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	SStaticModelVertexData vert_data;
	float stride = XM_2PI / Detail;
	for (uint8_t i = 0; i < Detail; ++i)
	{
		vert_data.Vertices.push_back(SStaticModelVertex(0, 0, 0, 0, 0, Color.x, Color.y, Color.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			Color.x, Color.y, Color.z, 1));
		vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			Color.x, Color.y, Color.z, 1));
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

void JWModel::MakeCube(float Size) noexcept
{
	m_RenderType = ERenderType::Model_Static;

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
	m_RenderType = ERenderType::Model_Static;

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
	m_RenderType = ERenderType::Model_Static;

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
	m_RenderType = ERenderType::Model_Static;

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

void JWModel::MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept
{
	m_RenderType = ERenderType::Model_Static;

	XMFLOAT3 ColorA{ 0, 0.5f, 0 };
	XMFLOAT3 ColorB{ 0, 0.5f, 1 };

	VerticalDetail = max(VerticalDetail, 4);
	HorizontalDetail = max(HorizontalDetail, 1);
	HorizontalDetail = (HorizontalDetail + 1) * 2;

	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	SStaticModelVertexData vert_data;
	float vert_stride = XM_2PI / HorizontalDetail; // Side circle
	float horz_stride = XM_2PI / VerticalDetail; // Top-down circle
	float start = ((static_cast<float>(HorizontalDetail) / 4.0f) - 1.0f);
	for (uint8_t i = 0; i < VerticalDetail; ++i)
	{
		// Up center (= sphere center)
		vert_data.Vertices.push_back(SStaticModelVertex(
			0,
			Radius,
			0,
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Up right down
		vert_data.Vertices.push_back(SStaticModelVertex(
			Radius * cosf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			0, 0,
			ColorB.x, ColorB.y, ColorB.z, 1));

		// Up left down
		vert_data.Vertices.push_back(SStaticModelVertex(
			Radius * cosf(horz_stride * i) * cosf(vert_stride * start),
			Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * i) * cosf(vert_stride * start),
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Down center (= sphere center)
		vert_data.Vertices.push_back(SStaticModelVertex(
			0,
			-Radius,
			0,
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Down right down
		vert_data.Vertices.push_back(SStaticModelVertex(
			Radius * cosf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			-Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			0, 0,

			ColorB.x, ColorB.y, ColorB.z, 1));

		// Down left down
		vert_data.Vertices.push_back(SStaticModelVertex(
			Radius * cosf(horz_stride * i) * cosf(vert_stride * start),
			-Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * i) * cosf(vert_stride * start),
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));
	}

	// if (HorizontalDetail == 4), which is the minimum size,
	// no sides are needed!
	if (HorizontalDetail > 4)
	{
		uint8_t real_horz_count = (HorizontalDetail / 2) - 1;

		for (uint8_t horz = 1; horz < real_horz_count; ++horz)
		{
			for (uint8_t vert = 0; vert < VerticalDetail; ++vert)
			{
				// Side #1 left up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start + 1 - horz)),
					Radius * sinf(vert_stride * (start + 1 - horz)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start + 1 - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 right up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start + 1 - horz)),
					Radius * sinf(vert_stride * (start + 1 - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start + 1 - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 left down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start + 1 - horz)),
					Radius * sinf(vert_stride * (start + 1 - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start + 1 - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorB.x, ColorB.y, ColorB.z, 1));

				// Side #2 left down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));
			}
		}
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

void JWModel::MakeCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept
{
	m_RenderType = ERenderType::Model_Static;

	XMFLOAT3 ColorA{ 0, 0.5f, 0 };
	XMFLOAT3 ColorB{ 0, 0.5f, 1 };

	VerticalDetail = max(VerticalDetail, 4);
	HorizontalDetail = max(HorizontalDetail, 1);

	// If HorizontalDetail is even number, there might be crack in the capsule,
	// so let's keep it odd number.
	if ((HorizontalDetail % 2) == 0)
	{
		++HorizontalDetail;
	}
	HorizontalDetail = (HorizontalDetail + 1) * 2;

	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	SStaticModelVertexData vert_data;
	float vert_stride = XM_2PI / HorizontalDetail; // Side circle
	float horz_stride = XM_2PI / VerticalDetail; // Top-down circle
	float start = ((static_cast<float>(HorizontalDetail) / 4.0f) - 1.0f);

	// Uppder hemisphere tip
	for (uint8_t i = 0; i < VerticalDetail; ++i)
	{
		// Up center (= sphere center)
		vert_data.Vertices.push_back(SStaticModelVertex(
			0,
			Radius,
			0,
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Up right down
		vert_data.Vertices.push_back(SStaticModelVertex(
			Radius * cosf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			0, 0,
			ColorB.x, ColorB.y, ColorB.z, 1));

		// Up left down
		vert_data.Vertices.push_back(SStaticModelVertex(
			Radius * cosf(horz_stride * i) * cosf(vert_stride * start),
			Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * i) * cosf(vert_stride * start),
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));
	}

	// if (HorizontalDetail == 4), which is the minimum size,
	// no sides are needed!
	if (HorizontalDetail > 4)
	{
		uint8_t real_horz_count = (HorizontalDetail / 2) - 1;
		uint8_t half_count = real_horz_count / 2;
		float side_offset_y{};

		// Uppder hemisphere
		for (uint8_t horz = 0; horz < half_count; ++horz)
		{
			for (uint8_t vert = 0; vert < VerticalDetail; ++vert)
			{
				// Side #1 left up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 right up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 left down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz - 1)),
					Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorB.x, ColorB.y, ColorB.z, 1));

				// Side #2 left down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				side_offset_y = Radius * sinf(vert_stride * (start - horz - 1));
			}
		}

		// Cylindrical side
		for (uint8_t i = 0; i < VerticalDetail; ++i)
		{
			// Side #1 (0 - 1 - 2)
			vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(horz_stride * i), side_offset_y - Height, Radius * sinf(horz_stride * i), 0, 0,
				ColorA.x * 0.9f, ColorA.y * 0.9f, ColorA.z * 0.9f, 1));
			vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(horz_stride * (i + 1)), side_offset_y - Height, Radius * sinf(horz_stride * (i + 1)), 0, 0,
				ColorA.x * 0.9f, ColorA.y * 0.9f, ColorA.z * 0.9f, 1));
			vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(horz_stride * i), side_offset_y, Radius * sinf(horz_stride * i), 0, 0,
				ColorA.x * 0.9f, ColorA.y * 0.9f, ColorA.z * 0.9f, 1));

			// Side #2 (1 - 3 - 2)
			vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(horz_stride * (i + 1)), side_offset_y - Height, Radius * sinf(horz_stride * (i + 1)), 0, 0,
				ColorB.x * 0.9f, ColorB.y * 0.9f, ColorB.z * 0.9f, 1));
			vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(horz_stride * (i + 1)), side_offset_y, Radius * sinf(horz_stride * (i + 1)), 0, 0,
				ColorB.x * 0.9f, ColorB.y * 0.9f, ColorB.z * 0.9f, 1));
			vert_data.Vertices.push_back(SStaticModelVertex(Radius * cosf(horz_stride * i), side_offset_y, Radius * sinf(horz_stride * i), 0, 0,
				ColorB.x * 0.9f, ColorB.y * 0.9f, ColorB.z * 0.9f, 1));
		}

		// Lower hemisphere
		for (uint8_t horz = half_count; horz < real_horz_count; ++horz)
		{
			for (uint8_t vert = 0; vert < VerticalDetail; ++vert)
			{
				// Side #1 left up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					-Height + Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 right up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					-Height + Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 left down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					-Height + Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right up
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					-Height + Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz - 1)),
					-Height + Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorB.x, ColorB.y, ColorB.z, 1));

				// Side #2 left down
				vert_data.Vertices.push_back(SStaticModelVertex(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					-Height + Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));
			}
		}
	}

	// Lower hemisphere tip
	for (uint8_t i = 0; i < VerticalDetail; ++i)
	{
		// Down center (= sphere center)
		vert_data.Vertices.push_back(SStaticModelVertex(
			0,
			-Height -Radius,
			0,
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Down right down
		vert_data.Vertices.push_back(SStaticModelVertex(
			Radius * cosf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			-Height -Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			0, 0,

			ColorB.x, ColorB.y, ColorB.z, 1));

		// Down left down
		vert_data.Vertices.push_back(SStaticModelVertex(
			Radius * cosf(horz_stride * i) * cosf(vert_stride * start),
			-Height -Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * i) * cosf(vert_stride * start),
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));
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
	if (m_RenderType != ERenderType::Invalid)
	{
		return;
	}

	m_RenderType = ERenderType::Model_Static;

	// Save the model data
	StaticModelData = ModelData;

	CreateModelVertexIndexBuffers();

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
	if (m_RenderType != ERenderType::Invalid)
	{
		return;
	}

	m_RenderType = ERenderType::Model_Line;

	NormalData = Model2Data;
	
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
		m_pDX->CreateStaticVertexBuffer(StaticModelData.VertexData.GetByteSize(), StaticModelData.VertexData.GetPtrData(), &ModelVertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(StaticModelData.IndexData.GetByteSize(), StaticModelData.IndexData.GetPtrData(), &ModelIndexBuffer);

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

auto JWModel::AddAnimationFromFile(STRING ModelFileName) noexcept->JWModel*
{
	if (m_RenderType == ERenderType::Model_Rigged)
	{
		JWAssimpLoader loader;
		loader.LoadAdditionalAnimationIntoRiggedModel(RiggedModelData, *m_pBaseDirectory + KAssetDirectory, ModelFileName);
	}

	return this;
}
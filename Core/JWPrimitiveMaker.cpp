#include "JWPrimitiveMaker.h"

using namespace JWEngine;


auto JWPrimitiveMaker::MakeTriangle(XMFLOAT3 A, XMFLOAT3 B, XMFLOAT3 C) noexcept->SModelData
{
	SModelData result{};

	XMFLOAT3 Color[3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	/*
	** Vertex
	*/
	result.VertexData.AddVertex(SVertexModel(A.x, A.y, A.z, Color[0].x, Color[0].y, Color[0].z, 1.0f));
	result.VertexData.AddVertex(SVertexModel(B.x, B.y, B.z, Color[1].x, Color[1].y, Color[1].z, 1.0f));
	result.VertexData.AddVertex(SVertexModel(C.x, C.y, C.z, Color[2].x, Color[2].y, Color[2].z, 1.0f));

	/*
	** Index
	*/
	result.IndexData.vIndices.push_back(SIndexTriangle(0, 1, 2));
	
	return result;
}

auto JWPrimitiveMaker::MakeQuad(XMFLOAT3 A, XMFLOAT3 B, XMFLOAT3 C, XMFLOAT3 D) noexcept->SModelData
{
	SModelData result{};

	XMFLOAT3 Color[4] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1} };

	/*
	** Vertex
	*/
	result.VertexData.AddVertex(SVertexModel(A.x, A.y, A.z, Color[0].x, Color[0].y, Color[0].z, 1.0f));
	result.VertexData.AddVertex(SVertexModel(B.x, B.y, B.z, Color[1].x, Color[1].y, Color[1].z, 1.0f));
	result.VertexData.AddVertex(SVertexModel(C.x, C.y, C.z, Color[2].x, Color[2].y, Color[2].z, 1.0f));
	result.VertexData.AddVertex(SVertexModel(D.x, D.y, D.z, Color[3].x, Color[3].y, Color[3].z, 1.0f));

	/*
	** Index
	*/
	result.IndexData.vIndices.push_back(SIndexTriangle(0, 1, 3));
	result.IndexData.vIndices.push_back(SIndexTriangle(1, 2, 3));

	return result;
}

auto JWPrimitiveMaker::MakeSquare(float Size, XMFLOAT2 UVMap) noexcept->SModelData
{
	SModelData result{};

	float half_size = Size / 2.0f;

	XMFLOAT3 Color[4] = { {1, 1, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	/*
	** Vertex
	*/
	// (LeftUp - RightUp - LeftDown - RightDown order)
	result.VertexData.AddVertex(SVertexModel(-half_size, 0, +half_size, 0, 0, Color[0].x, Color[0].y, Color[0].z, 1.0f));
	result.VertexData.AddVertex(SVertexModel(+half_size, 0, +half_size, UVMap.x, 0, Color[1].x, Color[1].y, Color[1].z, 1.0f));
	result.VertexData.AddVertex(SVertexModel(-half_size, 0, -half_size, 0, UVMap.y, Color[2].x, Color[2].y, Color[2].z, 1.0f));
	result.VertexData.AddVertex(SVertexModel(+half_size, 0, -half_size, UVMap.x, UVMap.y, Color[3].x, Color[3].y, Color[3].z, 1.0f));

	/*
	** Index
	*/
	for (unsigned int i = 0; i < result.VertexData.GetVertexCount() / 4; ++i)
	{
		// Clock-wise winding
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 4, i * 4 + 1, i * 4 + 2));
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 4 + 1, i * 4 + 3, i * 4 + 2));
	}

	return result;
}

auto JWPrimitiveMaker::MakeCircle(float Radius, uint8_t Detail) noexcept->SModelData
{
	SModelData result{};

	XMFLOAT3 Color{ 0, 0, 1 };

	Detail = max(Detail, 5);

	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	float stride = XM_2PI / Detail;
	for (uint8_t i = 0; i < Detail; ++i)
	{
		result.VertexData.AddVertex(SVertexModel(0, 0, 0, 0, 0, Color.x, Color.y, Color.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			Color.x, Color.y, Color.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			Color.x, Color.y, Color.z, 1));
	}

	/*
	** Index
	*/
	for (unsigned int i = 0; i < result.VertexData.GetVertexCount() / 3; ++i)
	{
		// Clock-wise winding
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 3, i * 3 + 2, i * 3 + 1));
	}

	return result;
}

auto JWPrimitiveMaker::MakeCube(float Size) noexcept->SModelData
{
	SModelData result{};

	float half_size = Size / 2.0f;

	XMFLOAT3 UpColor[4]
		= { {1, 1, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	XMFLOAT3 DownColor[4]
		= { {1, 1, 0}, {0, 1, 1}, {1, 0, 1}, {0, 0, 0} };

	/*
	** Vertex
	*/
	// Up (LeftUp - RightUp - LeftDown - RightDown order)
	result.VertexData.AddVertex(SVertexModel(-half_size, +half_size, +half_size, 0, 0, UpColor[0].x, UpColor[0].y, UpColor[0].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, +half_size, +half_size, 1, 0, UpColor[1].x, UpColor[1].y, UpColor[1].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_size, +half_size, -half_size, 0, 1, UpColor[2].x, UpColor[2].y, UpColor[2].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, +half_size, -half_size, 1, 1, UpColor[3].x, UpColor[3].y, UpColor[3].z, 1));

	// Down
	result.VertexData.AddVertex(SVertexModel(-half_size, -half_size, -half_size, 0, 0, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, -half_size, -half_size, 1, 0, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_size, -half_size, +half_size, 0, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, -half_size, +half_size, 1, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));

	// Front
	result.VertexData.AddVertex(SVertexModel(-half_size, +half_size, -half_size, 0, 0, UpColor[2].x, UpColor[2].y, UpColor[2].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, +half_size, -half_size, 1, 0, UpColor[3].x, UpColor[3].y, UpColor[3].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_size, -half_size, -half_size, 0, 1, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, -half_size, -half_size, 1, 1, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));

	// Right
	result.VertexData.AddVertex(SVertexModel(+half_size, +half_size, -half_size, 0, 0, UpColor[3].x, UpColor[3].y, UpColor[3].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, +half_size, +half_size, 1, 0, UpColor[1].x, UpColor[1].y, UpColor[1].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, -half_size, -half_size, 0, 1, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, -half_size, +half_size, 1, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));

	// Back
	result.VertexData.AddVertex(SVertexModel(+half_size, +half_size, +half_size, 0, 0, UpColor[1].x, UpColor[1].y, UpColor[1].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_size, +half_size, +half_size, 1, 0, UpColor[0].x, UpColor[0].y, UpColor[0].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_size, -half_size, +half_size, 0, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_size, -half_size, +half_size, 1, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));

	// Left
	result.VertexData.AddVertex(SVertexModel(-half_size, +half_size, +half_size, 0, 0, UpColor[0].x, UpColor[0].y, UpColor[0].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_size, +half_size, -half_size, 1, 0, UpColor[2].x, UpColor[2].y, UpColor[2].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_size, -half_size, +half_size, 0, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_size, -half_size, -half_size, 1, 1, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));

	/*
	** Index
	*/
	for (unsigned int i = 0; i < result.VertexData.GetVertexCount() / 4; ++i)
	{
		// Clock-wise winding
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 4, i * 4 + 1, i * 4 + 2));
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 4 + 1, i * 4 + 3, i * 4 + 2));
	}

	return result;
}

auto JWPrimitiveMaker::MakePyramid(float Height, float Width) noexcept->SModelData
{
	SModelData result{};

	float half_width = Width / 2.0f;

	XMFLOAT3 UpColor{ 1, 1, 1 };
	XMFLOAT3 DownColor[4]
		= { {1, 1, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	/*
	** Vertex
	*/
	// Down (LeftUp - RightUp - LeftDown - RightDown order)
	result.VertexData.AddVertex(SVertexModel(-half_width, 0, +half_width, 0, 0, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_width, 0, +half_width, 1, 0, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_width, 0, -half_width, 0, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_width, 0, -half_width, 1, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));

	// Front (Tip - LeftDown - RightDown order)
	result.VertexData.AddVertex(SVertexModel(0, Height, 0, 0.5f, 1, UpColor.x, UpColor.y, UpColor.z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_width, 0, -half_width, 0, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_width, 0, -half_width, 1, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));

	// Right
	result.VertexData.AddVertex(SVertexModel(0, Height, 0, 0.5f, 1, UpColor.x, UpColor.y, UpColor.z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_width, 0, -half_width, 0, 1, DownColor[3].x, DownColor[3].y, DownColor[3].z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_width, 0, +half_width, 1, 1, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));

	// Back
	result.VertexData.AddVertex(SVertexModel(0, Height, 0, 0.5f, 1, UpColor.x, UpColor.y, UpColor.z, 1));
	result.VertexData.AddVertex(SVertexModel(+half_width, 0, +half_width, 0, 1, DownColor[1].x, DownColor[1].y, DownColor[1].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_width, 0, +half_width, 1, 1, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));

	// Left
	result.VertexData.AddVertex(SVertexModel(0, Height, 0, 0.5f, 1, UpColor.x, UpColor.y, UpColor.z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_width, 0, +half_width, 0, 1, DownColor[0].x, DownColor[0].y, DownColor[0].z, 1));
	result.VertexData.AddVertex(SVertexModel(-half_width, 0, -half_width, 1, 1, DownColor[2].x, DownColor[2].y, DownColor[2].z, 1));

	/*
	** Index
	*/
	result.IndexData.vIndices.push_back(SIndexTriangle(0, 1, 2));
	result.IndexData.vIndices.push_back(SIndexTriangle(1, 3, 2));

	int ind_offset = 4;
	for (unsigned int i = 0; i < (result.VertexData.GetVertexCount() - 4) / 3; ++i)
	{
		// Clock-wise winding
		result.IndexData.vIndices.push_back(SIndexTriangle(ind_offset + i * 3, ind_offset + i * 3 + 2, ind_offset + i * 3 + 1));
	}

	return result;
}

auto JWPrimitiveMaker::MakeCone(float Height, float Radius, uint8_t Detail) noexcept->SModelData
{
	SModelData result{};

	XMFLOAT3 UpColor{ 0, 0, 1 };
	XMFLOAT3 DownColor{ 0, 1, 0 };

	Detail = max(Detail, 5);

	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	float stride = XM_2PI / Detail;
	for (uint8_t i = 0; i < Detail; ++i)
	{
		// Down
		result.VertexData.AddVertex(SVertexModel(0, 0, 0, 0, 0, DownColor.x, DownColor.y, DownColor.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));

		// Side
		result.VertexData.AddVertex(SVertexModel(0, Height, 0, 0, 0, UpColor.x, UpColor.y, UpColor.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));
	}

	/*
	** Index
	*/
	for (unsigned int i = 0; i < result.VertexData.GetVertexCount() / 3; ++i)
	{
		// Clock-wise winding
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 3, i * 3 + 2, i * 3 + 1));
	}

	return result;
}

auto JWPrimitiveMaker::MakeCylinder(float Height, float Radius, uint8_t Detail) noexcept->SModelData
{
	SModelData result{};

	XMFLOAT3 UpColor{ 0, 0, 1 };
	XMFLOAT3 DownColor{ 0, 1, 0 };

	Detail = max(Detail, 5);

	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	float stride = XM_2PI / Detail;
	for (uint8_t i = 0; i < Detail; ++i)
	{
		// Up
		result.VertexData.AddVertex(SVertexModel(0, Height, 0, 0, 0, UpColor.x, UpColor.y, UpColor.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * i), Height, Radius * sinf(stride * i), 0, 0,
			UpColor.x, UpColor.y, UpColor.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * (i + 1)), Height, Radius * sinf(stride * (i + 1)), 0, 0,
			UpColor.x, UpColor.y, UpColor.z, 1));

		// Down
		result.VertexData.AddVertex(SVertexModel(0, 0, 0, 0, 0, DownColor.x, DownColor.y, DownColor.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			DownColor.x, DownColor.y, DownColor.z, 1));

		// Side #1 (0 - 1 - 2)
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * i), Height, Radius * sinf(stride * i), 0, 0,
			UpColor.x * 0.9f, UpColor.y * 0.9f, UpColor.z * 0.9f, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * (i + 1)), Height, Radius * sinf(stride * (i + 1)), 0, 0,
			UpColor.x * 0.9f, UpColor.y * 0.9f, UpColor.z * 0.9f, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x * 0.9f, DownColor.y * 0.9f, DownColor.z * 0.9f, 1));

		// Side #2 (1 - 3 - 2)
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * (i + 1)), Height, Radius * sinf(stride * (i + 1)), 0, 0,
			UpColor.x * 0.9f, UpColor.y * 0.9f, UpColor.z * 0.9f, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * (i + 1)), 0, Radius * sinf(stride * (i + 1)), 0, 0,
			DownColor.x * 0.9f, DownColor.y * 0.9f, DownColor.z * 0.9f, 1));
		result.VertexData.AddVertex(SVertexModel(Radius * cosf(stride * i), 0, Radius * sinf(stride * i), 0, 0,
			DownColor.x * 0.9f, DownColor.y * 0.9f, DownColor.z * 0.9f, 1));
	}

	/*
	** Index
	*/
	for (unsigned int i = 0; i < result.VertexData.GetVertexCount() / 3; ++i)
	{
		// Clock-wise winding
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 3, i * 3 + 2, i * 3 + 1));
	}

	return result;
}

auto JWPrimitiveMaker::MakeSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->SModelData
{
	SModelData result{};

	XMFLOAT3 ColorA{ 0, 1, 0 };
	XMFLOAT3 ColorB{ 0, 1, 1 };

	VerticalDetail = max(VerticalDetail, 4);
	HorizontalDetail = max(HorizontalDetail, 1);
	HorizontalDetail = (HorizontalDetail + 1) * 2;

	// (Straight right -> Counter-clokwise)
	/*
	** Vertex
	*/
	float vert_stride = XM_2PI / HorizontalDetail; // Side circle
	float horz_stride = XM_2PI / VerticalDetail; // Top-down circle
	float vert_start = ((static_cast<float>(HorizontalDetail) / 4.0f) - 1.0f);
	for (uint8_t vert = 0; vert < VerticalDetail; ++vert)
	{
		// Up center (= sphere center)
		result.VertexData.AddVertex(SVertexModel(
			0,
			Radius,
			0,
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Up left down
		result.VertexData.AddVertex(SVertexModel(
			Radius * cosf(horz_stride * vert) * cosf(vert_stride * vert_start),
			Radius * sinf(vert_stride * vert_start),
			Radius * sinf(horz_stride * vert) * cosf(vert_stride * vert_start),
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Up right down
		result.VertexData.AddVertex(SVertexModel(
			Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * vert_start),
			Radius * sinf(vert_stride * vert_start),
			Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * vert_start),
			0, 0,
			ColorB.x, ColorB.y, ColorB.z, 1));

		// Down center (= sphere center)
		result.VertexData.AddVertex(SVertexModel(
			0,
			-Radius,
			0,
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Down right down
		result.VertexData.AddVertex(SVertexModel(
			Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * vert_start),
			-Radius * sinf(vert_stride * vert_start),
			Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * vert_start),
			0, 0,
			ColorB.x, ColorB.y, ColorB.z, 1));

		// Down left down
		result.VertexData.AddVertex(SVertexModel(
			Radius * cosf(horz_stride * vert) * cosf(vert_stride * vert_start),
			-Radius * sinf(vert_stride * vert_start),
			Radius * sinf(horz_stride * vert) * cosf(vert_stride * vert_start),
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
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * vert) * cosf(vert_stride * (vert_start + 1 - horz)),
					Radius * sinf(vert_stride * (vert_start + 1 - horz)),
					Radius * sinf(horz_stride * vert) * cosf(vert_stride * (vert_start + 1 - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 left down
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * vert) * cosf(vert_stride * (vert_start - horz)),
					Radius * sinf(vert_stride * (vert_start - horz)),
					Radius * sinf(horz_stride * vert) * cosf(vert_stride * (vert_start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 right up
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (vert_start + 1 - horz)),
					Radius * sinf(vert_stride * (vert_start + 1 - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (vert_start + 1 - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right up
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (vert_start + 1 - horz)),
					Radius * sinf(vert_stride * (vert_start + 1 - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (vert_start + 1 - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 left down
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * vert) * cosf(vert_stride * (vert_start - horz)),
					Radius * sinf(vert_stride * (vert_start - horz)),
					Radius * sinf(horz_stride * vert) * cosf(vert_stride * (vert_start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right down
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (vert_start - horz)),
					Radius * sinf(vert_stride * (vert_start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (vert_start - horz)),
					0, 0,
					ColorB.x, ColorB.y, ColorB.z, 1));
			}
		}
	}

	/*
	** Index
	*/
	for (unsigned int i = 0; i < result.VertexData.GetVertexCount() / 3; ++i)
	{
		// Clock-wise winding
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 3, i * 3 + 2, i * 3 + 1));
	}

	return result;
}

auto JWPrimitiveMaker::MakeCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->SModelData
{
	SModelData result{};

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
	float vert_stride = XM_2PI / HorizontalDetail; // Side circle
	float horz_stride = XM_2PI / VerticalDetail; // Top-down circle
	float start = ((static_cast<float>(HorizontalDetail) / 4.0f) - 1.0f);

	// Uppder hemisphere tip
	for (uint8_t i = 0; i < VerticalDetail; ++i)
	{
		// Up center (= sphere center)
		result.VertexData.AddVertex(SVertexModel(
			0,
			Radius,
			0,
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Up right down
		result.VertexData.AddVertex(SVertexModel(
			Radius * cosf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			0, 0,
			ColorB.x, ColorB.y, ColorB.z, 1));

		// Up left down
		result.VertexData.AddVertex(SVertexModel(
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
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 right up
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 left down
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right up
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right down
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz - 1)),
					Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorB.x, ColorB.y, ColorB.z, 1));

				// Side #2 left down
				result.VertexData.AddVertex(SVertexModel(
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
			result.VertexData.AddVertex(SVertexModel(Radius * cosf(horz_stride * i), side_offset_y - Height, Radius * sinf(horz_stride * i), 0, 0,
				ColorA.x * 0.9f, ColorA.y * 0.9f, ColorA.z * 0.9f, 1));
			result.VertexData.AddVertex(SVertexModel(Radius * cosf(horz_stride * (i + 1)), side_offset_y - Height, Radius * sinf(horz_stride * (i + 1)), 0, 0,
				ColorA.x * 0.9f, ColorA.y * 0.9f, ColorA.z * 0.9f, 1));
			result.VertexData.AddVertex(SVertexModel(Radius * cosf(horz_stride * i), side_offset_y, Radius * sinf(horz_stride * i), 0, 0,
				ColorA.x * 0.9f, ColorA.y * 0.9f, ColorA.z * 0.9f, 1));

			// Side #2 (1 - 3 - 2)
			result.VertexData.AddVertex(SVertexModel(Radius * cosf(horz_stride * (i + 1)), side_offset_y - Height, Radius * sinf(horz_stride * (i + 1)), 0, 0,
				ColorB.x * 0.9f, ColorB.y * 0.9f, ColorB.z * 0.9f, 1));
			result.VertexData.AddVertex(SVertexModel(Radius * cosf(horz_stride * (i + 1)), side_offset_y, Radius * sinf(horz_stride * (i + 1)), 0, 0,
				ColorB.x * 0.9f, ColorB.y * 0.9f, ColorB.z * 0.9f, 1));
			result.VertexData.AddVertex(SVertexModel(Radius * cosf(horz_stride * i), side_offset_y, Radius * sinf(horz_stride * i), 0, 0,
				ColorB.x * 0.9f, ColorB.y * 0.9f, ColorB.z * 0.9f, 1));
		}

		// Lower hemisphere
		for (uint8_t horz = half_count; horz < real_horz_count; ++horz)
		{
			for (uint8_t vert = 0; vert < VerticalDetail; ++vert)
			{
				// Side #1 left up
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					-Height + Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 right up
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					-Height + Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #1 left down
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					-Height + Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right up
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					-Height + Radius * sinf(vert_stride * (start - horz)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz)),
					0, 0,
					ColorA.x, ColorA.y, ColorA.z, 1));

				// Side #2 right down
				result.VertexData.AddVertex(SVertexModel(
					Radius * cosf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz - 1)),
					-Height + Radius * sinf(vert_stride * (start - horz - 1)),
					Radius * sinf(horz_stride * (vert + 1)) * cosf(vert_stride * (start - horz - 1)),
					0, 0,
					ColorB.x, ColorB.y, ColorB.z, 1));

				// Side #2 left down
				result.VertexData.AddVertex(SVertexModel(
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
		result.VertexData.AddVertex(SVertexModel(
			0,
			-Height - Radius,
			0,
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));

		// Down right down
		result.VertexData.AddVertex(SVertexModel(
			Radius * cosf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			-Height - Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * (i + 1)) * cosf(vert_stride * start),
			0, 0,

			ColorB.x, ColorB.y, ColorB.z, 1));

		// Down left down
		result.VertexData.AddVertex(SVertexModel(
			Radius * cosf(horz_stride * i) * cosf(vert_stride * start),
			-Height - Radius * sinf(vert_stride * start),
			Radius * sinf(horz_stride * i) * cosf(vert_stride * start),
			0, 0,
			ColorA.x, ColorA.y, ColorA.z, 1));
	}

	/*
	** Index
	*/
	for (unsigned int i = 0; i < result.VertexData.GetVertexCount() / 3; ++i)
	{
		// Clock-wise winding
		result.IndexData.vIndices.push_back(SIndexTriangle(i * 3, i * 3 + 2, i * 3 + 1));
	}

	return result;
}
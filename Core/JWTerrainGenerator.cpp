#include "JWTerrainGenerator.h"
#include "JWDX.h"

using namespace JWEngine;

void JWTerrainGenerator::Create(JWDX& DX, const STRING& BaseDirectory) noexcept
{
	m_pDX = &DX;
	m_BaseDirectory = BaseDirectory;
}

inline auto JWTerrainGenerator::ConvertR8G8B8ToFloat(unsigned char R, unsigned char G, unsigned char B, float division_factor) noexcept->float
{
	return static_cast<float>(R + G * 256 + B * 65536) / division_factor;
}

inline auto JWTerrainGenerator::ConvertR8ToFloat(unsigned char R, float division_factor) noexcept->float
{
	return static_cast<float>(R) / division_factor;
}

void JWTerrainGenerator::LoadR8G8B8A8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
	float HeightFactor, SModelData& OutModelData) noexcept
{
	uint32_t color_count = 4;
	uint32_t actual_texture_width = TextureWidth;
	if (TextureWidth % 2)
	{
		++actual_texture_width;
	}
	uint32_t array_size = color_count * actual_texture_width * TextureHeight;
	unsigned char* data = new unsigned char[array_size];
	assert(data);

	// Map the readable texture
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(Texture, 0, D3D11_MAP_READ, 0, &mapped_subresource)))
	{
		memcpy(data, mapped_subresource.pData, sizeof(unsigned char) * array_size);
		m_pDX->GetDeviceContext()->Unmap(Texture, 0);
	}

	float		v_x{};
	float		v_z{};
	float		v_y[4]{};

	// Vertex
	for (uint32_t z = 0; z < TextureHeight - 1; ++z)
	{
		for (uint32_t x = 0; x < TextureWidth - 1; ++x)
		{
			v_x = static_cast<float>(x);
			v_z = -static_cast<float>(z);

			// Ignore alpha value
			v_y[0] = ConvertR8G8B8ToFloat(
				data[(z * actual_texture_width * color_count) + (x * color_count)],
				data[(z * actual_texture_width * color_count) + (x * color_count) + 1],
				data[(z * actual_texture_width * color_count) + (x * color_count) + 2],
				HeightFactor
			);

			v_y[1] = ConvertR8G8B8ToFloat(
				data[(z * actual_texture_width * color_count) + ((x + 1) * color_count)],
				data[(z * actual_texture_width * color_count) + ((x + 1) * color_count) + 1],
				data[(z * actual_texture_width * color_count) + ((x + 1) * color_count) + 2],
				HeightFactor
			);

			v_y[2] = ConvertR8G8B8ToFloat(
				data[((z + 1) * actual_texture_width * color_count) + (x * color_count)],
				data[((z + 1) * actual_texture_width * color_count) + (x * color_count) + 1],
				data[((z + 1) * actual_texture_width * color_count) + (x * color_count) + 2],
				HeightFactor
			);

			v_y[3] = ConvertR8G8B8ToFloat(
				data[((z + 1) * actual_texture_width * color_count) + ((x + 1) * color_count)],
				data[((z + 1) * actual_texture_width * color_count) + ((x + 1) * color_count) + 1],
				data[((z + 1) * actual_texture_width * color_count) + ((x + 1) * color_count) + 2],
				HeightFactor
			);

			OutModelData.VertexData.AddVertex(SVertexModel(v_x		 , v_y[0], v_z		 , 0, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + 1.0f, v_y[1], v_z		 , 1, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x		 , v_y[2], v_z - 1.0f, 0, 1));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + 1.0f, v_y[3], v_z - 1.0f, 1, 1));
		}
	}

	JW_DELETE_ARRAY(data);
}

void JWTerrainGenerator::LoadGray8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
	float HeightFactor, SModelData& OutModelData) noexcept
{
	uint32_t color_count = 1;
	uint32_t array_size = color_count * TextureWidth * TextureHeight;
	unsigned char* data = new unsigned char[array_size];

	// Map the readable texture
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(Texture, 0, D3D11_MAP_READ, 0, &mapped_subresource)))
	{
		memcpy(data, mapped_subresource.pData, sizeof(unsigned char) * array_size);
		m_pDX->GetDeviceContext()->Unmap(Texture, 0);
	}

	float		v_x{};
	float		v_z{};
	float		v_y[4]{};

	// Vertex
	for (uint32_t z = 0; z < TextureHeight - 1; ++z)
	{
		for (uint32_t x = 0; x < TextureWidth - 1; ++x)
		{
			v_x = static_cast<float>(x);
			v_z = -static_cast<float>(z);

			// Ignore alpha value
			v_y[0] = ConvertR8ToFloat(
				data[(z * TextureWidth * color_count) + (x * color_count)],
				HeightFactor
			);

			v_y[1] = ConvertR8ToFloat(
				data[(z * TextureWidth * color_count) + ((x + 1) * color_count)],
				HeightFactor
			);

			v_y[2] = ConvertR8ToFloat(
				data[((z + 1) * TextureWidth * color_count) + (x * color_count)],
				HeightFactor
			);

			v_y[3] = ConvertR8ToFloat(
				data[((z + 1) * TextureWidth * color_count) + ((x + 1) * color_count)],
				HeightFactor
			);

			OutModelData.VertexData.AddVertex(SVertexModel(v_x, v_y[0], v_z, 0, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + 1, v_y[1], v_z, 1, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x, v_y[2], v_z - 1, 0, 1));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + 1, v_y[3], v_z - 1, 1, 1));
		}
	}

	JW_DELETE_ARRAY(data);
}

auto JWTerrainGenerator::GenerateTerrainFromFile(const STRING& FileName, float HeightFactor) noexcept->STerrainData
{
	STerrainData terrain_data{};
	SModelData model_data{};

	auto w_fn = StringToWstring(m_BaseDirectory + KAssetDirectory + FileName);

	STextureData texture_data{};
	auto& texture = texture_data.Texture;
	auto& texture_srv = texture_data.TextureSRV;
	auto& texture_size = texture_data.TextureSize;

	// Load texture from file.
	CreateWICTextureFromFile(m_pDX->GetDevice(), w_fn.c_str(), (ID3D11Resource**)&texture, &texture_srv, 0);

	if (texture)
	{
		// Get texture description from loaded texture.
		D3D11_TEXTURE2D_DESC loaded_texture_desc{};
		texture->GetDesc(&loaded_texture_desc);
		texture_size.Width = loaded_texture_desc.Width;
		texture_size.Height = loaded_texture_desc.Height;

		terrain_data.TerrainSizeX = texture_size.Width - 1;
		terrain_data.TerrainSizeZ = texture_size.Height - 1;

		// Modify texture description
		loaded_texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		loaded_texture_desc.Usage = D3D11_USAGE_STAGING;
		loaded_texture_desc.BindFlags = 0;

		// Create texture for reading
		ID3D11Texture2D* readable_texture{};
		m_pDX->GetDevice()->CreateTexture2D(&loaded_texture_desc, nullptr, &readable_texture);

		if (readable_texture == nullptr)
		{
			JW_ERROR_ABORT("Failed to create the readable texture.");
		}

		// Copy texture data
		m_pDX->GetDeviceContext()->CopyResource(readable_texture, texture);

		bool are_vertices_loaded{ false };
		if (loaded_texture_desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			LoadR8G8B8A8UnormData(readable_texture, texture_size.Width, texture_size.Height, HeightFactor, model_data);
			are_vertices_loaded = true;
		}
		else if (loaded_texture_desc.Format == DXGI_FORMAT_R8_UNORM)
		{
			LoadGray8UnormData(readable_texture, texture_size.Width, texture_size.Height, HeightFactor, model_data);
			are_vertices_loaded = true;
		}

		if (are_vertices_loaded)
		{
			// Calculate the whole bounding sphere's radius and offset.
			{
				XMFLOAT3 max_pos{ -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX };
				XMFLOAT3 min_pos{ D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX , D3D11_FLOAT32_MAX };

				for (auto& iter : model_data.VertexData.vVerticesModel)
				{
					max_pos.x = max(max_pos.x, iter.Position.x);
					max_pos.y = max(max_pos.y, iter.Position.y);
					max_pos.z = max(max_pos.z, iter.Position.z);

					min_pos.x = min(min_pos.x, iter.Position.x);
					min_pos.y = min(min_pos.y, iter.Position.y);
					min_pos.z = min(min_pos.z, iter.Position.z);
				}

				float dx = (max_pos.x - min_pos.x);
				float dy = (max_pos.y - min_pos.y);
				float dz = (max_pos.z - min_pos.z);
				float radius = sqrtf(dx * dx + dy * dy + dz * dz) / 2.0f;

				terrain_data.WholeBoundingSphereRadius = radius;
				terrain_data.WholeBoundingSphereOffset.x = dx / 2.0f;
				terrain_data.WholeBoundingSphereOffset.y = dy / 2.0f;
				terrain_data.WholeBoundingSphereOffset.z = -dz / 2.0f;
			}

			// Index
			for (uint32_t i = 0; i < (texture_size.Width - 1) * (texture_size.Height - 1); ++i)
			{
				model_data.IndexData.vIndices.push_back(SIndexTriangle(i * 4	, i * 4 + 1, i * 4 + 2));
				model_data.IndexData.vIndices.push_back(SIndexTriangle(i * 4 + 1, i * 4 + 3, i * 4 + 2));
			}

			// Compute normals, tangents, bitangents
			auto & v_indices = model_data.IndexData.vIndices;
			auto & v_vertices = model_data.VertexData.vVerticesModel;
			XMVECTOR v_0{}, v_1{}, v_2{};
			XMVECTOR e_0{}, e_1{}, e_2{};
			XMVECTOR normal{};
			float du_0{}, du_1{}, dv_0{}, dv_1{};
			float det{};
			float length{};
			XMFLOAT3 tangent{}, bitangent{};
			
			for (auto& iter : v_indices)
			{
				// #0 Normal

				// Get vertices of this face
				v_0 = XMLoadFloat3(&v_vertices[iter._0].Position);
				v_1 = XMLoadFloat3(&v_vertices[iter._1].Position);
				v_2 = XMLoadFloat3(&v_vertices[iter._2].Position);

				// Get edges to compute Normal vector.
				e_0 = v_1 - v_0;
				e_1 = v_2 - v_0;

				// Calculate Normal vector.
				normal = XMVector3Normalize(XMVector3Cross(e_0, e_1));

				XMStoreFloat3(&v_vertices[iter._0].Normal, normal);
				XMStoreFloat3(&v_vertices[iter._1].Normal, normal);
				XMStoreFloat3(&v_vertices[iter._2].Normal, normal);


				// #1 Tangent & Bitangent

				// Additional edge for computing Tangent & Bitangent vectors.
				e_2 = v_2 - v_1;

				// u = TexCoord.x
				du_0 = v_vertices[iter._1].TexCoord.x - v_vertices[iter._0].TexCoord.x;
				du_1 = v_vertices[iter._2].TexCoord.x - v_vertices[iter._1].TexCoord.x;

				// v = TexCoord.y
				dv_0 = v_vertices[iter._1].TexCoord.y - v_vertices[iter._0].TexCoord.y;
				dv_1 = v_vertices[iter._2].TexCoord.y - v_vertices[iter._1].TexCoord.y;

				// Get inverse matrix of 2x2 UV matrix
				det = du_0 * dv_1 - du_1 * dv_0;
				
				tangent.x = det * (XMVectorGetX(e_0) * dv_1 + XMVectorGetX(e_2) * -dv_0);
				tangent.y = det * (XMVectorGetY(e_0) * dv_1 + XMVectorGetY(e_2) * -dv_0);
				tangent.z = det * (XMVectorGetZ(e_0) * dv_1 + XMVectorGetZ(e_2) * -dv_0);
				length = sqrtf(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);
				
				tangent.x = tangent.x / length;
				tangent.y = tangent.y / length;
				tangent.z = tangent.z / length;
				
				bitangent.x = det * (XMVectorGetX(e_0) * -du_1 + XMVectorGetX(e_2) * du_0);
				bitangent.y = det * (XMVectorGetY(e_0) * -du_1 + XMVectorGetY(e_2) * du_0);
				bitangent.z = det * (XMVectorGetZ(e_0) * -du_1 + XMVectorGetZ(e_2) * du_0);
				length = sqrtf(bitangent.x * bitangent.x + bitangent.y * bitangent.y + bitangent.z * bitangent.z);

				bitangent.x = bitangent.x / length;
				bitangent.y = bitangent.y / length;
				bitangent.z = bitangent.z / length;

				v_vertices[iter._0].Tangent = tangent;
				v_vertices[iter._1].Tangent = tangent;
				v_vertices[iter._2].Tangent = tangent;

				v_vertices[iter._0].Bitangent = bitangent;
				v_vertices[iter._1].Bitangent = bitangent;
				v_vertices[iter._2].Bitangent = bitangent;
			}
		}
		else
		{
			JW_ERROR_ABORT("No data loaded.");
		}

		JW_RELEASE(readable_texture);
	}

	// Release all resources
	JW_RELEASE(texture);
	JW_RELEASE(texture_srv);

	// Create quad tree [2, 16] x [2, 16]
	auto& tree = terrain_data.QuadTree;
	tree.push_back(STerrainQuadTreeNode(0, -1));
	tree[0].SizeX = terrain_data.TerrainSizeX;
	tree[0].SizeZ = terrain_data.TerrainSizeZ;

	BuildQuadTree(terrain_data, 0);
	BuildQuadTreeMesh(terrain_data, model_data);

	return terrain_data;
}

void JWTerrainGenerator::BuildQuadTree(STerrainData& TerrainData, int32_t CurrentNodeID) noexcept
{
	auto& tree = TerrainData.QuadTree;
	auto size = static_cast<int32_t>(tree.size() - 1);
	
	if ((tree[CurrentNodeID].SizeX > KMaximumNodeSizeX) || (tree[CurrentNodeID].SizeZ > KMaximumNodeSizeZ) &&
		(tree[CurrentNodeID].SizeX > KMinimumNodeSizeX) && (tree[CurrentNodeID].SizeZ > KMinimumNodeSizeZ))
	{
		// Add 4 children
		tree.push_back(STerrainQuadTreeNode(size + 1, CurrentNodeID));
		tree.push_back(STerrainQuadTreeNode(size + 2, CurrentNodeID));
		tree.push_back(STerrainQuadTreeNode(size + 3, CurrentNodeID));
		tree.push_back(STerrainQuadTreeNode(size + 4, CurrentNodeID));

		// Save children's ID into current node.
		tree[CurrentNodeID].ChildrenID[0] = size + 1;
		tree[CurrentNodeID].ChildrenID[1] = size + 2;
		tree[CurrentNodeID].ChildrenID[2] = size + 3;
		tree[CurrentNodeID].ChildrenID[3] = size + 4;

		auto& child_0 = tree[tree[CurrentNodeID].ChildrenID[0]];
		auto& child_1 = tree[tree[CurrentNodeID].ChildrenID[1]];
		auto& child_2 = tree[tree[CurrentNodeID].ChildrenID[2]];
		auto& child_3 = tree[tree[CurrentNodeID].ChildrenID[3]];

		child_0.SizeX = child_2.SizeX = tree[CurrentNodeID].SizeX / 2;
		child_1.SizeX = child_3.SizeX = tree[CurrentNodeID].SizeX - child_0.SizeX;

		child_0.SizeZ = child_1.SizeZ = tree[CurrentNodeID].SizeZ / 2;
		child_2.SizeZ = child_3.SizeZ = tree[CurrentNodeID].SizeZ - child_0.SizeZ;

		child_0.StartX = tree[CurrentNodeID].StartX;
		child_0.StartZ = tree[CurrentNodeID].StartZ;

		child_1.StartX = child_0.StartX + child_0.SizeX;
		child_1.StartZ = child_0.StartZ;

		child_2.StartX = child_0.StartX;
		child_2.StartZ = child_0.StartZ + child_0.SizeZ;

		child_3.StartX = child_0.StartX + child_0.SizeX;
		child_3.StartZ = child_0.StartZ + child_0.SizeZ;

		// Build tree recursively.
		BuildQuadTree(TerrainData, size + 1);
		BuildQuadTree(TerrainData, size + 2);
		BuildQuadTree(TerrainData, size + 3);
		BuildQuadTree(TerrainData, size + 4);
	}
}

void JWTerrainGenerator::BuildQuadTreeMesh(STerrainData& TerrainData, const SModelData& ModelData) noexcept
{
	auto& tree = TerrainData.QuadTree;

	for (auto& iter : tree)
	{
		if (iter.ChildrenID[0] == -1)
		{
			// If this is a leaf node, build vertices.

			iter.IsMeshNode = true;
			++TerrainData.TerrainMeshCount;
			
			// Vertex
			uint32_t vertex_offset{ iter.StartZ * (TerrainData.TerrainSizeX - 1) * 4 + iter.StartX * 4 };
			XMFLOAT3 max_pos{ -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX };
			XMFLOAT3 min_pos{ D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX , D3D11_FLOAT32_MAX };

			for (uint32_t z = iter.StartZ; z < iter.StartZ + iter.SizeZ; ++z)
			{
				for (uint32_t x = iter.StartX; x < iter.StartX + iter.SizeX; ++x)
				{
					vertex_offset = z * TerrainData.TerrainSizeX * 4 + x * 4;
					
					for (int i = 0; i < 4; ++i)
					{
						iter.VertexData.AddVertex(ModelData.VertexData.vVerticesModel[vertex_offset]);

						max_pos.x = max(ModelData.VertexData.vVerticesModel[vertex_offset].Position.x, max_pos.x);
						max_pos.y = max(ModelData.VertexData.vVerticesModel[vertex_offset].Position.y, max_pos.y);
						max_pos.z = max(ModelData.VertexData.vVerticesModel[vertex_offset].Position.z, max_pos.z);

						min_pos.x = min(ModelData.VertexData.vVerticesModel[vertex_offset].Position.x, min_pos.x);
						min_pos.y = min(ModelData.VertexData.vVerticesModel[vertex_offset].Position.y, min_pos.y);
						min_pos.z = min(ModelData.VertexData.vVerticesModel[vertex_offset].Position.z, min_pos.z);

						++vertex_offset;
					}
				}
			}

			float dx = (max_pos.x - min_pos.x);
			float dy = (max_pos.y - min_pos.y);
			float dz = (max_pos.z - min_pos.z);
			float radius = sqrtf(dx * dx + dy * dy + dz * dz) / 2.0f;

			TerrainData.SubBoundingSpheres.push_back(SBoundingSphereData());
			iter.SubBoundingSphereID = static_cast<uint32_t>(TerrainData.SubBoundingSpheres.size() - 1);

			TerrainData.SubBoundingSpheres[iter.SubBoundingSphereID].Center = 
				XMVectorSet(min_pos.x + dx / 2.0f, min_pos.y + dy / 2.0f, min_pos.z + dz / 2.0f, 1);
			TerrainData.SubBoundingSpheres[iter.SubBoundingSphereID].Radius = radius;
			
			// Index
			for (uint32_t i = 0; i < iter.SizeX * iter.SizeZ; ++i)
			{
				iter.IndexData.vIndices.push_back(SIndexTriangle(i * 4	  , i * 4 + 1, i * 4 + 2));
				iter.IndexData.vIndices.push_back(SIndexTriangle(i * 4 + 1, i * 4 + 3, i * 4 + 2));
			}

			// Create vertex buffer
			m_pDX->CreateStaticVertexBuffer(
				iter.VertexData.GetVertexModelByteSize(), iter.VertexData.GetVertexModelPtrData(), &iter.VertexBuffer);

			// Create index buffer
			m_pDX->CreateIndexBuffer(iter.IndexData.GetByteSize(), iter.IndexData.GetPtrData(), &iter.IndexBuffer);

			// Calculate Normal vector representations
			// Calculate normal line positions
			size_t iterator_vertex{};
			XMFLOAT3 second_vertex_position{};
			for (const auto& vertex : ModelData.VertexData.vVerticesModel)
			{
				second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
				second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
				second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

				iter.NormalData.VertexData.AddVertex(SVertexModel(vertex.Position, KDefaultColorNormals));
				iter.NormalData.VertexData.AddVertex(SVertexModel(second_vertex_position, KDefaultColorNormals));
				iter.NormalData.IndexData.vIndices.push_back(
					SIndexLine(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
				++iterator_vertex;
			}

			// Create vertex buffer for normals
			m_pDX->CreateStaticVertexBuffer(
				iter.NormalData.VertexData.GetVertexModelByteSize(), iter.NormalData.VertexData.GetVertexModelPtrData(), &iter.NormalVertexBuffer);

			// Create index buffer for normals
			m_pDX->CreateIndexBuffer(iter.NormalData.IndexData.GetByteSize(), iter.NormalData.IndexData.GetPtrData(), &iter.NormalIndexBuffer);

		}
	}
}
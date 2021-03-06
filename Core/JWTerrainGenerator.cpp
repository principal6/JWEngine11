#include "JWTerrainGenerator.h"
#include "JWDX.h"
#include "../TinyXml2/tinyxml2.h"

using namespace JWEngine;

void JWTerrainGenerator::Create(JWDX& DX, const STRING& BaseDirectory) noexcept
{
	m_pDX = &DX;
	m_BaseDirectory = BaseDirectory;
}

inline auto JWTerrainGenerator::ConvertR8G8B8ToFloat(unsigned char R, unsigned char G, unsigned char B, float factor) noexcept->float
{
	return (static_cast<float>(R + G * UINT8_MAX + B * UINT16_MAX) / (UINT16_MAX * 2)) * factor;
}

inline auto JWTerrainGenerator::ConvertR8ToFloat(unsigned char R, float factor) noexcept->float
{
	return (static_cast<float>(R) / UINT8_MAX) * factor;
}

inline auto JWTerrainGenerator::ConvertR16ToFloat(unsigned short R, float factor) noexcept->float
{
	return (static_cast<float>(R) / UINT16_MAX) * factor;
}

PRIVATE void JWTerrainGenerator::LoadR8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
	float HeightFactor, float XYSizeFactor, SModelData& OutModelData, SVertexMap& OutVertexMap) noexcept
{
	static constexpr auto pixel_byte_size = sizeof(unsigned char);
	static constexpr uint32_t color_count = 1;
	uint32_t texture_row_size{};
	uint32_t array_size{};
	unsigned char* data{};

	// Map the readable texture
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(Texture, 0, D3D11_MAP_READ, 0, &mapped_subresource)))
	{
		texture_row_size = mapped_subresource.RowPitch / (pixel_byte_size * color_count);
		array_size = mapped_subresource.DepthPitch / pixel_byte_size;
		data = new unsigned char[array_size];

		memcpy(data, mapped_subresource.pData, pixel_byte_size * array_size);
		m_pDX->GetDeviceContext()->Unmap(Texture, 0);
	}

	float		v_x{};
	float		v_z{};
	float		v_y[4]{};
	uint32_t	v_map_id[4]{};

	// Vertex
	for (uint32_t z = 0; z < TextureHeight - 1; ++z)
	{
		for (uint32_t x = 0; x < TextureWidth - 1; ++x)
		{
			v_x = static_cast<float>(x) * XYSizeFactor;
			v_z = -static_cast<float>(z) * XYSizeFactor;

			// Ignore alpha value
			v_y[0] = ConvertR8ToFloat(
				data[(z * texture_row_size * color_count) + (x * color_count)],
				HeightFactor
			);

			v_y[1] = ConvertR8ToFloat(
				data[(z * texture_row_size * color_count) + ((x + 1) * color_count)],
				HeightFactor
			);

			v_y[2] = ConvertR8ToFloat(
				data[((z + 1) * texture_row_size * color_count) + (x * color_count)],
				HeightFactor
			);

			v_y[3] = ConvertR8ToFloat(
				data[((z + 1) * texture_row_size * color_count) + ((x + 1) * color_count)],
				HeightFactor
			);

			OutModelData.VertexData.AddVertex(SVertexModel(v_x				 , v_y[0], v_z				 , 0, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + XYSizeFactor, v_y[1], v_z				 , 1, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x				 , v_y[2], v_z - XYSizeFactor, 0, 1));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + XYSizeFactor, v_y[3], v_z - XYSizeFactor, 1, 1));

			v_map_id[0] = x + z * TextureWidth;
			v_map_id[1] = (x + 1) + z * TextureWidth;
			v_map_id[2] = x + (z + 1) * TextureWidth;
			v_map_id[3] = (x + 1) + (z + 1) * TextureWidth;

			OutVertexMap[v_map_id[0]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 4));
			OutVertexMap[v_map_id[1]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 3));
			OutVertexMap[v_map_id[2]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 2));
			OutVertexMap[v_map_id[3]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 1));
		}
	}

	JW_DELETE_ARRAY(data);
}

PRIVATE void JWTerrainGenerator::LoadR16UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
	float HeightFactor, float XYSizeFactor, SModelData& OutModelData, SVertexMap& OutVertexMap) noexcept
{
	static constexpr auto pixel_byte_size = sizeof(unsigned short);
	static constexpr uint32_t color_count = 1;
	uint32_t texture_row_size{};
	uint32_t array_size{};
	unsigned short* data{};

	// Map the readable texture
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(Texture, 0, D3D11_MAP_READ, 0, &mapped_subresource)))
	{
		texture_row_size = mapped_subresource.RowPitch / (pixel_byte_size * color_count);
		array_size = mapped_subresource.DepthPitch / pixel_byte_size;
		data = new unsigned short[array_size];

		memcpy(data, mapped_subresource.pData, pixel_byte_size * array_size);
		m_pDX->GetDeviceContext()->Unmap(Texture, 0);
	}

	float		v_x{};
	float		v_z{};
	float		v_y[4]{};
	uint32_t	v_map_id[4]{};

	// Vertex
	for (uint32_t z = 0; z < TextureHeight - 1; ++z)
	{
		for (uint32_t x = 0; x < TextureWidth - 1; ++x)
		{
			v_x = static_cast<float>(x) * XYSizeFactor;
			v_z = -static_cast<float>(z) * XYSizeFactor;

			// Ignore alpha value
			v_y[0] = ConvertR16ToFloat(
				data[(z * texture_row_size * color_count) + (x * color_count)],
				HeightFactor
			);

			v_y[1] = ConvertR16ToFloat(
				data[(z * texture_row_size * color_count) + ((x + 1) * color_count)],
				HeightFactor
			);

			v_y[2] = ConvertR16ToFloat(
				data[((z + 1) * texture_row_size * color_count) + (x * color_count)],
				HeightFactor
			);

			v_y[3] = ConvertR16ToFloat(
				data[((z + 1) * texture_row_size * color_count) + ((x + 1) * color_count)],
				HeightFactor
			);

			OutModelData.VertexData.AddVertex(SVertexModel(v_x, v_y[0], v_z, 0, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + XYSizeFactor, v_y[1], v_z, 1, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x, v_y[2], v_z - XYSizeFactor, 0, 1));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + XYSizeFactor, v_y[3], v_z - XYSizeFactor, 1, 1));

			v_map_id[0] = x + z * TextureWidth;
			v_map_id[1] = (x + 1) + z * TextureWidth;
			v_map_id[2] = x + (z + 1) * TextureWidth;
			v_map_id[3] = (x + 1) + (z + 1) * TextureWidth;

			OutVertexMap[v_map_id[0]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 4));
			OutVertexMap[v_map_id[1]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 3));
			OutVertexMap[v_map_id[2]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 2));
			OutVertexMap[v_map_id[3]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 1));
		}
	}

	JW_DELETE_ARRAY(data);
}

PRIVATE void JWTerrainGenerator::LoadR8G8B8A8UnormData(ID3D11Texture2D* Texture, uint32_t TextureWidth, uint32_t TextureHeight,
	float HeightFactor, float XYSizeFactor, SModelData& OutModelData, SVertexMap& OutVertexMap) noexcept
{
	static constexpr auto pixel_byte_size = sizeof(unsigned char);
	static constexpr uint32_t color_count = 4;
	uint32_t texture_row_size{};
	uint32_t array_size{};
	unsigned char* data{};

	// Map the readable texture
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(Texture, 0, D3D11_MAP_READ, 0, &mapped_subresource)))
	{
		texture_row_size = mapped_subresource.RowPitch / (pixel_byte_size * color_count);
		array_size = mapped_subresource.DepthPitch / pixel_byte_size;
		data = new unsigned char[array_size];

		memcpy(data, mapped_subresource.pData, pixel_byte_size * array_size);
		m_pDX->GetDeviceContext()->Unmap(Texture, 0);
	}

	float		v_x{};
	float		v_z{};
	float		v_y[4]{};
	uint32_t	v_map_id[4]{};

	// Vertex
	for (uint32_t z = 0; z < TextureHeight - 1; ++z)
	{
		for (uint32_t x = 0; x < TextureWidth - 1; ++x)
		{
			v_x = static_cast<float>(x) * XYSizeFactor;
			v_z = -static_cast<float>(z) * XYSizeFactor;

			// Ignore alpha value
			v_y[0] = ConvertR8G8B8ToFloat(
				data[(z * texture_row_size * color_count) + (x * color_count)],
				data[(z * texture_row_size * color_count) + (x * color_count) + 1],
				data[(z * texture_row_size * color_count) + (x * color_count) + 2],
				HeightFactor
			);

			v_y[1] = ConvertR8G8B8ToFloat(
				data[(z * texture_row_size * color_count) + ((x + 1) * color_count)],
				data[(z * texture_row_size * color_count) + ((x + 1) * color_count) + 1],
				data[(z * texture_row_size * color_count) + ((x + 1) * color_count) + 2],
				HeightFactor
			);

			v_y[2] = ConvertR8G8B8ToFloat(
				data[((z + 1) * texture_row_size * color_count) + (x * color_count)],
				data[((z + 1) * texture_row_size * color_count) + (x * color_count) + 1],
				data[((z + 1) * texture_row_size * color_count) + (x * color_count) + 2],
				HeightFactor
			);

			v_y[3] = ConvertR8G8B8ToFloat(
				data[((z + 1) * texture_row_size * color_count) + ((x + 1) * color_count)],
				data[((z + 1) * texture_row_size * color_count) + ((x + 1) * color_count) + 1],
				data[((z + 1) * texture_row_size * color_count) + ((x + 1) * color_count) + 2],
				HeightFactor
			);

			OutModelData.VertexData.AddVertex(SVertexModel(v_x				 , v_y[0], v_z				 , 0, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + XYSizeFactor, v_y[1], v_z				 , 1, 0));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x				 , v_y[2], v_z - XYSizeFactor, 0, 1));
			OutModelData.VertexData.AddVertex(SVertexModel(v_x + XYSizeFactor, v_y[3], v_z - XYSizeFactor, 1, 1));

			v_map_id[0] = x + z * TextureWidth;
			v_map_id[1] = (x + 1) + z * TextureWidth;
			v_map_id[2] = x + (z + 1) * TextureWidth;
			v_map_id[3] = (x + 1) + (z + 1) * TextureWidth;

			OutVertexMap[v_map_id[0]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 4));
			OutVertexMap[v_map_id[1]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 3));
			OutVertexMap[v_map_id[2]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 2));
			OutVertexMap[v_map_id[3]].AddVertexID(static_cast<int32_t>(OutModelData.VertexData.vVerticesModel.size() - 1));
		}
	}

	JW_DELETE_ARRAY(data);
}

auto JWTerrainGenerator::GenerateTerrainFromHeightMap(const STRING& HeightMapFN, float HeightFactor, float XYSizeFactor) noexcept->STerrainData
{
	SVertexMap vertex_map{};
	STerrainData terrain_data{};
	SModelData model_data{};

	auto w_fn = StringToWstring(m_BaseDirectory + KAssetDirectory + HeightMapFN);

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
		terrain_data.HeightFactor = HeightFactor;
		terrain_data.XYSizeFactor = XYSizeFactor;

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

		// Load vertices from the texture
		vertex_map.reserve(static_cast<uint64_t>(texture_size.Width) * texture_size.Height);
		vertex_map.resize(static_cast<uint64_t>(texture_size.Width) * texture_size.Height);

		bool are_vertices_loaded{ false };
		if (loaded_texture_desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			LoadR8G8B8A8UnormData(readable_texture, texture_size.Width, texture_size.Height, HeightFactor, XYSizeFactor, model_data, vertex_map);
			are_vertices_loaded = true;
		}
		else if (loaded_texture_desc.Format == DXGI_FORMAT_R8_UNORM)
		{
			LoadR8UnormData(readable_texture, texture_size.Width, texture_size.Height, HeightFactor, XYSizeFactor, model_data, vertex_map);
			are_vertices_loaded = true;
		}
		else if (loaded_texture_desc.Format == DXGI_FORMAT_R16_UNORM)
		{
			LoadR16UnormData(readable_texture, texture_size.Width, texture_size.Height, HeightFactor, XYSizeFactor, model_data, vertex_map);
			are_vertices_loaded = true;
		}

		if (are_vertices_loaded)
		{
			// Calculate the whole bounding sphere's center and radius.
			{
				XMVECTOR max_v{ -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, 1.0f };
				XMVECTOR min_v{ D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX , D3D11_FLOAT32_MAX, 1.0f };

				for (auto& iter : model_data.VertexData.vVerticesModel)
				{
					max_v = XMVectorMax(max_v, iter.Position);
					min_v = XMVectorMin(min_v, iter.Position);
				}

				auto d = max_v - min_v;
				auto r = XMVector3Length(d) / 2.0f;
				
				// Bounding sphere
				terrain_data.WholeBoundingSphere.Center = XMVectorSet(
					XMVectorGetX(d) / 2.0f,
					XMVectorGetY(d) / 2.0f,
					-XMVectorGetZ(d) / 2.0f,
					0.0f);

				terrain_data.WholeBoundingSphere.Radius = XMVectorGetX(r);

				// Bounding ellipsoid
				/*
				terrain_data.WholeBoundingEllipsoid.Offset = XMVectorSet(
					XMVectorGetX(d) / 2.0f,
					XMVectorGetY(d) / 2.0f,
					-XMVectorGetZ(d) / 2.0f,
					0.0f);
				
				terrain_data.WholeBoundingEllipsoid.RadiusX = XMVectorGetX(r);
				terrain_data.WholeBoundingEllipsoid.RadiusY = XMVectorGetX(r);
				terrain_data.WholeBoundingEllipsoid.RadiusZ = XMVectorGetX(r);
				*/
			}

			// Index
			for (uint32_t i = 0; i < (texture_size.Width - 1) * (texture_size.Height - 1); ++i)
			{
				model_data.IndexData.vFaces.push_back(SIndexTriangle(i * 4	, i * 4 + 1, i * 4 + 2));
				model_data.IndexData.vFaces.push_back(SIndexTriangle(i * 4 + 1, i * 4 + 3, i * 4 + 2));
			}

			// Compute normals, tangents, bitangents
			auto & v_indices = model_data.IndexData.vFaces;
			auto & v_vertices = model_data.VertexData.vVerticesModel;
			XMVECTOR e_0{}, e_1{}, e_2{};
			XMVECTOR normal{};
			float du_0{}, du_1{}, dv_0{}, dv_1{};
			float det{};
			float length{};
			XMVECTOR tangent{}, bitangent{};
			
			for (auto& iter : v_indices)
			{
				// #0 Normal

				// Get vertices of this face
				const auto& v_0 = v_vertices[iter._0].Position;
				const auto& v_1 = v_vertices[iter._1].Position;
				const auto& v_2 = v_vertices[iter._2].Position;

				// Calculate Normal vector.
				e_0 = v_1 - v_0;
				e_1 = v_2 - v_0;
				normal = XMVector3Normalize(XMVector3Cross(e_0, e_1));
				v_vertices[iter._0].Normal = normal;
				v_vertices[iter._1].Normal = normal;
				v_vertices[iter._2].Normal = normal;
				

				// #1 Tangent & Bitangent
				
				// Additional edge for computing Tangent & Bitangent vectors.
				e_2 = v_2 - v_1;

				// u = TexCoord.x
				du_0 = XMVectorGetX(v_vertices[iter._1].TexCoord - v_vertices[iter._0].TexCoord);
				du_1 = XMVectorGetX(v_vertices[iter._2].TexCoord - v_vertices[iter._1].TexCoord);

				// v = TexCoord.y
				dv_0 = XMVectorGetY(v_vertices[iter._1].TexCoord - v_vertices[iter._0].TexCoord);
				dv_1 = XMVectorGetY(v_vertices[iter._2].TexCoord - v_vertices[iter._1].TexCoord);

				// Get inverse matrix of 2x2 UV matrix
				det = du_0 * dv_1 - du_1 * dv_0;
				
				tangent = XMVector3Normalize(det * (e_0 * dv_1 + e_2 * -dv_0));
				bitangent = XMVector3Normalize(det * (e_0 * -du_1 + e_2 * du_0));

				v_vertices[iter._0].Tangent = tangent;
				v_vertices[iter._1].Tangent = tangent;
				v_vertices[iter._2].Tangent = tangent;

				v_vertices[iter._0].Bitangent = bitangent;
				v_vertices[iter._1].Bitangent = bitangent;
				v_vertices[iter._2].Bitangent = bitangent;
			}

			// Calculate vertex normals
			for (auto& iter : vertex_map)
			{
				XMVECTOR averaged_normal{};

				for (uint32_t i = 0; i < iter.VertexCount; ++i)
				{
					// For every face that contains the same vertex
					averaged_normal += v_vertices[iter.VertexID[i]].Normal;
				}

				// Normalize
				averaged_normal = XMVector3Normalize(averaged_normal);
				
				for (uint32_t i = 0; i < iter.VertexCount; ++i)
				{
					v_vertices[iter.VertexID[i]].Normal = averaged_normal;
				}
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

void JWTerrainGenerator::SaveTerrainAsTRN(const STRING& TRNFileName, const STerrainData& TerrainData) noexcept
{
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc{};

	auto root = doc.NewElement("jw_terrain_root");

	auto terrain_info = doc.NewElement("terrain_info");
	terrain_info->SetAttribute("size_x", TerrainData.TerrainSizeX);
	terrain_info->SetAttribute("size_z", TerrainData.TerrainSizeZ);
	terrain_info->SetAttribute("height_factor", TerrainData.HeightFactor);
	terrain_info->SetAttribute("xy_size_factor", TerrainData.XYSizeFactor);
	terrain_info->SetAttribute("whole_bounding_sphere_center_x", XMVectorGetX(TerrainData.WholeBoundingSphere.Center));
	terrain_info->SetAttribute("whole_bounding_sphere_center_y", XMVectorGetY(TerrainData.WholeBoundingSphere.Center));
	terrain_info->SetAttribute("whole_bounding_sphere_center_z", XMVectorGetZ(TerrainData.WholeBoundingSphere.Center));
	terrain_info->SetAttribute("whole_bounding_sphere_radius", TerrainData.WholeBoundingSphere.Radius);
	/*
	terrain_info->SetAttribute("whole_bounding_ellipsoid_offset_x", XMVectorGetX(TerrainData.WholeBoundingEllipsoid.Offset));
	terrain_info->SetAttribute("whole_bounding_ellipsoid_offset_y", XMVectorGetY(TerrainData.WholeBoundingEllipsoid.Offset));
	terrain_info->SetAttribute("whole_bounding_ellipsoid_offset_z", XMVectorGetZ(TerrainData.WholeBoundingEllipsoid.Offset));
	terrain_info->SetAttribute("whole_bounding_ellipsoid_radius_x", TerrainData.WholeBoundingEllipsoid.RadiusX);
	terrain_info->SetAttribute("whole_bounding_ellipsoid_radius_y", TerrainData.WholeBoundingEllipsoid.RadiusY);
	terrain_info->SetAttribute("whole_bounding_ellipsoid_radius_z", TerrainData.WholeBoundingEllipsoid.RadiusZ);
	*/

	auto quad_tree = doc.NewElement("quad_tree");
	quad_tree->SetAttribute("node_count", static_cast<int>(TerrainData.QuadTree.size()));

	for (auto& iter : TerrainData.QuadTree)
	{
		auto node = doc.NewElement("node");
		node->SetAttribute("node_id", iter.NodeID);
		node->SetAttribute("parent_id", iter.ParentID);
		node->SetAttribute("children_id_0", iter.ChildrenID[0]);
		node->SetAttribute("children_id_1", iter.ChildrenID[1]);
		node->SetAttribute("children_id_2", iter.ChildrenID[2]);
		node->SetAttribute("children_id_3", iter.ChildrenID[3]);
		node->SetAttribute("start_x", iter.StartX);
		node->SetAttribute("start_z", iter.StartZ);
		node->SetAttribute("size_x", iter.SizeX);
		node->SetAttribute("size_z", iter.SizeZ);
		node->SetAttribute("has_meshes", iter.HasMeshes);
		node->SetAttribute("sub_bounding_sphere_id", iter.SubBoundingVolumeID);

		if (iter.HasMeshes)
		{
			auto vertices = doc.NewElement("vertices");
			vertices->SetAttribute("vertex_count", static_cast<int>(iter.VertexData.vVerticesModel.size()));
			for (auto& vertex_iter : iter.VertexData.vVerticesModel)
			{
				auto vertex = doc.NewElement("vertex");

				auto position = doc.NewElement("position");
				position->SetAttribute("x", XMVectorGetX(vertex_iter.Position));
				position->SetAttribute("y", XMVectorGetY(vertex_iter.Position));
				position->SetAttribute("z", XMVectorGetZ(vertex_iter.Position));

				auto texcoord = doc.NewElement("texcoord");
				position->SetAttribute("u", XMVectorGetX(vertex_iter.TexCoord));
				position->SetAttribute("v", XMVectorGetY(vertex_iter.TexCoord));
				
				auto normal = doc.NewElement("normal");
				normal->SetAttribute("x", XMVectorGetX(vertex_iter.Normal));
				normal->SetAttribute("y", XMVectorGetY(vertex_iter.Normal));
				normal->SetAttribute("z", XMVectorGetZ(vertex_iter.Normal));

				auto tangent = doc.NewElement("tangent");
				tangent->SetAttribute("x", XMVectorGetX(vertex_iter.Tangent));
				tangent->SetAttribute("y", XMVectorGetY(vertex_iter.Tangent));
				tangent->SetAttribute("z", XMVectorGetZ(vertex_iter.Tangent));
				
				auto bitangent = doc.NewElement("bitangent");
				bitangent->SetAttribute("x", XMVectorGetX(vertex_iter.Bitangent));
				bitangent->SetAttribute("y", XMVectorGetY(vertex_iter.Bitangent));
				bitangent->SetAttribute("z", XMVectorGetZ(vertex_iter.Bitangent));
				
				vertex->InsertEndChild(position);
				vertex->InsertEndChild(texcoord);
				vertex->InsertEndChild(normal);
				vertex->InsertEndChild(tangent);
				vertex->InsertEndChild(bitangent);

				vertices->InsertEndChild(vertex);
			}

			auto faces = doc.NewElement("faces");
			faces->SetAttribute("face_count", static_cast<int>(iter.IndexData.vFaces.size()));
			for (auto& face_iter : iter.IndexData.vFaces)
			{
				auto face = doc.NewElement("face");
				face->SetAttribute("_0", static_cast<int>(face_iter._0));
				face->SetAttribute("_1", static_cast<int>(face_iter._1));
				face->SetAttribute("_2", static_cast<int>(face_iter._2));
				
				faces->InsertEndChild(face);
			}

			node->InsertEndChild(vertices);
			node->InsertEndChild(faces);
		}
		
		quad_tree->InsertEndChild(node);
	}

	auto sub_bounding_spheres = doc.NewElement("sub_bounding_spheres");
	sub_bounding_spheres->SetAttribute("count", static_cast<int>(TerrainData.SubBoundingSpheres.size()));

	for (auto& iter : TerrainData.SubBoundingSpheres)
	{
		auto sub_bounding_sphere = doc.NewElement("sub_bounding_sphere");

		auto center = doc.NewElement("center");
		center->SetAttribute("x", XMVectorGetX(iter.Center));
		center->SetAttribute("y", XMVectorGetY(iter.Center));
		center->SetAttribute("z", XMVectorGetZ(iter.Center));

		auto radius = doc.NewElement("radius");
		radius->SetText(iter.Radius);

		sub_bounding_sphere->InsertEndChild(center);
		sub_bounding_sphere->InsertEndChild(radius);

		sub_bounding_spheres->InsertEndChild(sub_bounding_sphere);
	}

	root->InsertEndChild(terrain_info);
	root->InsertEndChild(quad_tree);
	root->InsertEndChild(sub_bounding_spheres);
	
	doc.InsertFirstChild(root);
	doc.SaveFile((m_BaseDirectory + KAssetDirectory + TRNFileName).c_str());
}

auto JWTerrainGenerator::LoadTerrainFromTRN(const STRING& TRNFileName) noexcept->STerrainData
{
	STerrainData terrain_data{};
	SModelData model_data{};

	using namespace tinyxml2;
	tinyxml2::XMLDocument doc{};
	doc.LoadFile((m_BaseDirectory + KAssetDirectory + TRNFileName).c_str());

	auto root = doc.FirstChildElement();

	auto terrain_info = root->FirstChildElement();
	terrain_data.TerrainSizeX = terrain_info->IntAttribute("size_x");
	terrain_data.TerrainSizeZ = terrain_info->IntAttribute("size_z");
	terrain_data.HeightFactor = terrain_info->FloatAttribute("height_factor");
	terrain_data.XYSizeFactor = terrain_info->FloatAttribute("xy_size_factor");

	terrain_data.WholeBoundingSphere.Center = XMVectorSet(
		terrain_info->FloatAttribute("whole_bounding_sphere_center_x"),
		terrain_info->FloatAttribute("whole_bounding_sphere_center_y"),
		terrain_info->FloatAttribute("whole_bounding_sphere_center_z"),
		0.0f);
	terrain_data.WholeBoundingSphere.Radius = terrain_info->FloatAttribute("whole_bounding_sphere_radius");

	auto quad_tree = terrain_info->NextSiblingElement();
	auto node_count = quad_tree->IntAttribute("node_count");
	if (node_count)
	{
		auto xml_node = quad_tree->FirstChildElement();

		for (int i = 0; i < node_count; ++i)
		{
			terrain_data.QuadTree.push_back(STerrainQuadTreeNode());
			auto& current_node = terrain_data.QuadTree[terrain_data.QuadTree.size() - 1];

			current_node.NodeID = xml_node->IntAttribute("node_id");
			current_node.ParentID = xml_node->IntAttribute("parent_id");
			current_node.ChildrenID[0] = xml_node->IntAttribute("children_id_0");
			current_node.ChildrenID[1] = xml_node->IntAttribute("children_id_1");
			current_node.ChildrenID[2] = xml_node->IntAttribute("children_id_2");
			current_node.ChildrenID[3] = xml_node->IntAttribute("children_id_3");
			current_node.StartX = xml_node->IntAttribute("start_x");
			current_node.StartZ = xml_node->IntAttribute("start_z");
			current_node.SizeX = xml_node->IntAttribute("size_x");
			current_node.SizeZ = xml_node->IntAttribute("size_z");
			current_node.HasMeshes = xml_node->BoolAttribute("has_meshes");
			current_node.SubBoundingVolumeID = xml_node->IntAttribute("sub_bounding_sphere_id");

			if (current_node.HasMeshes)
			{
				auto vertices = xml_node->FirstChildElement();
				auto vertex_count = vertices->IntAttribute("vertex_count");
				auto xml_vertex = vertices->FirstChildElement();
				for (int i = 0; i < vertex_count; ++i)
				{
					SVertexModel current_vertex{};

					auto position = xml_vertex->FirstChildElement();
					current_vertex.Position =
						XMVectorSet(position->FloatAttribute("x"), position->FloatAttribute("y"), position->FloatAttribute("z"), 1.0f);
					
					auto texcoord = position->NextSiblingElement();
					current_vertex.TexCoord =
						XMVectorSet(position->FloatAttribute("u"), position->FloatAttribute("v"), 0, 0);

					auto normal = texcoord->NextSiblingElement();
					current_vertex.Normal =
						XMVectorSet(normal->FloatAttribute("x"), normal->FloatAttribute("y"), normal->FloatAttribute("z"), 0.0f);
					
					auto tangent = normal->NextSiblingElement();
					current_vertex.Tangent =
						XMVectorSet(tangent->FloatAttribute("x"), tangent->FloatAttribute("y"), tangent->FloatAttribute("z"), 0.0f);

					auto bitangent = tangent->NextSiblingElement();
					current_vertex.Bitangent =
						XMVectorSet(bitangent->FloatAttribute("x"), bitangent->FloatAttribute("y"), bitangent->FloatAttribute("z"), 0.0f);

					current_node.VertexData.AddVertex(current_vertex);

					xml_vertex = xml_vertex->NextSiblingElement();
				}

				auto faces = vertices->NextSiblingElement();
				auto face_count = faces->IntAttribute("face_count");
				auto xml_face = faces->FirstChildElement();
				for (int i = 0; i < face_count; ++i)
				{
					SIndexTriangle current_face = SIndexTriangle(
						xml_face->IntAttribute("_0"),
						xml_face->IntAttribute("_1"),
						xml_face->IntAttribute("_2"));

					current_node.IndexData.vFaces.emplace_back(current_face);

					xml_face = xml_face->NextSiblingElement();
				}

				// Create vertex buffer
				m_pDX->CreateStaticVertexBuffer(
					current_node.VertexData.GetVertexModelByteSize(), current_node.VertexData.GetVertexModelPtrData(), &current_node.VertexBuffer);

				// Create index buffer
				m_pDX->CreateIndexBuffer(current_node.IndexData.GetByteSize(), current_node.IndexData.GetPtrData(), &current_node.IndexBuffer);
			}

			xml_node = xml_node->NextSiblingElement();
		}
	}

	auto sub_bounding_spheres = quad_tree->NextSiblingElement();
	auto sub_bounding_sphere_count = sub_bounding_spheres->IntAttribute("count");
	if (sub_bounding_sphere_count)
	{
		auto xml_sub_bounding_sphere = sub_bounding_spheres->FirstChildElement();

		for (int i = 0; i < sub_bounding_sphere_count; ++i)
		{
			SBoundingSphereData curr_sub_bounding_sphere{};

			auto center = xml_sub_bounding_sphere->FirstChildElement();
			curr_sub_bounding_sphere.Center = XMVectorSet(
				center->FloatAttribute("x"),
				center->FloatAttribute("y"),
				center->FloatAttribute("z"),
				0.0f);

			auto radius = center->NextSiblingElement();
			curr_sub_bounding_sphere.Radius = radius->FloatText();

			terrain_data.SubBoundingSpheres.emplace_back(curr_sub_bounding_sphere);

			xml_sub_bounding_sphere = xml_sub_bounding_sphere->NextSiblingElement();
		}
	}

	return terrain_data;
}

void JWTerrainGenerator::BuildQuadTree(STerrainData& TerrainData, int32_t CurrentNodeID) noexcept
{
	auto& tree = TerrainData.QuadTree;
	auto size = static_cast<int32_t>(tree.size() - 1);
	
	if ((tree[CurrentNodeID].SizeX > KMaximumNodeSize) || (tree[CurrentNodeID].SizeZ > KMaximumNodeSize) &&
		(tree[CurrentNodeID].SizeX > KMinimumNodeSize) && (tree[CurrentNodeID].SizeZ > KMinimumNodeSize))
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
			iter.HasMeshes = true;
			
			// Vertex
			uint32_t vertex_offset{ iter.StartZ * (TerrainData.TerrainSizeX - 1) * 4 + iter.StartX * 4 };
			XMVECTOR max_v{ XMVectorSet(-D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, 1.0f) };
			XMVECTOR min_v{ XMVectorSet(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX , D3D11_FLOAT32_MAX, 1.0f) };
			
			for (uint32_t z = iter.StartZ; z < iter.StartZ + iter.SizeZ; ++z)
			{
				for (uint32_t x = iter.StartX; x < iter.StartX + iter.SizeX; ++x)
				{
					vertex_offset = z * TerrainData.TerrainSizeX * 4 + x * 4;
					
					for (int i = 0; i < 4; ++i)
					{
						iter.VertexData.AddVertex(ModelData.VertexData.vVerticesModel[vertex_offset]);

						max_v = XMVectorMax(max_v, ModelData.VertexData.vVerticesModel[vertex_offset].Position);
						min_v = XMVectorMin(min_v, ModelData.VertexData.vVerticesModel[vertex_offset].Position);

						++vertex_offset;
					}
				}
			}

			auto d = max_v - min_v;
			auto r = XMVector3Length(d) / 2.0f;
			auto center = min_v + d / 2.0f;
			
			TerrainData.SubBoundingSpheres.push_back(SBoundingSphereData());
			iter.SubBoundingVolumeID = static_cast<uint32_t>(TerrainData.SubBoundingSpheres.size() - 1);

			TerrainData.SubBoundingSpheres[iter.SubBoundingVolumeID].Center = center;
			TerrainData.SubBoundingSpheres[iter.SubBoundingVolumeID].Radius = XMVectorGetX(r);
			
			// Index
			for (uint32_t i = 0; i < iter.SizeX * iter.SizeZ; ++i)
			{
				iter.IndexData.vFaces.push_back(SIndexTriangle(i * 4	, i * 4 + 1, i * 4 + 2));
				iter.IndexData.vFaces.push_back(SIndexTriangle(i * 4 + 1, i * 4 + 3, i * 4 + 2));
			}

			// Create vertex buffer
			m_pDX->CreateStaticVertexBuffer(
				iter.VertexData.GetVertexModelByteSize(), iter.VertexData.GetVertexModelPtrData(), &iter.VertexBuffer);

			// Create index buffer
			m_pDX->CreateIndexBuffer(iter.IndexData.GetByteSize(), iter.IndexData.GetPtrData(), &iter.IndexBuffer);
		}
	}
}
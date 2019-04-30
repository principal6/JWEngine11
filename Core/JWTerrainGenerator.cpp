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
			v_y[0] = ConvertR8G8B8ToFloat(
				data[(z * TextureWidth * color_count) + (x * color_count)],
				data[(z * TextureWidth * color_count) + (x * color_count) + 1],
				data[(z * TextureWidth * color_count) + (x * color_count) + 2],
				HeightFactor
			);

			v_y[1] = ConvertR8G8B8ToFloat(
				data[(z * TextureWidth * color_count) + ((x + 1) * color_count)],
				data[(z * TextureWidth * color_count) + ((x + 1) * color_count) + 1],
				data[(z * TextureWidth * color_count) + ((x + 1) * color_count) + 2],
				HeightFactor
			);

			v_y[2] = ConvertR8G8B8ToFloat(
				data[((z + 1) * TextureWidth * color_count) + (x * color_count)],
				data[((z + 1) * TextureWidth * color_count) + (x * color_count) + 1],
				data[((z + 1) * TextureWidth * color_count) + (x * color_count) + 2],
				HeightFactor
			);

			v_y[3] = ConvertR8G8B8ToFloat(
				data[((z + 1) * TextureWidth * color_count) + ((x + 1) * color_count)],
				data[((z + 1) * TextureWidth * color_count) + ((x + 1) * color_count) + 1],
				data[((z + 1) * TextureWidth * color_count) + ((x + 1) * color_count) + 2],
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

auto JWTerrainGenerator::GenerateTerrainFromFile(const STRING& FileName, float HeightFactor) noexcept->SModelData
{
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

		// Modify texture description
		loaded_texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		loaded_texture_desc.Usage = D3D11_USAGE_STAGING;
		loaded_texture_desc.BindFlags = 0;

		// Create texture for reading
		ID3D11Texture2D* readable_texture{};
		m_pDX->GetDevice()->CreateTexture2D(&loaded_texture_desc, nullptr, &readable_texture);

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

	return model_data;
}
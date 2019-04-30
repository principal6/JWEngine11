#include "JWTerrainGenerator.h"
#include "JWDX.h"

using namespace JWEngine;

void JWTerrainGenerator::Create(JWDX& DX, const STRING& BaseDirectory) noexcept
{
	m_pDX = &DX;
	m_BaseDirectory = BaseDirectory;
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

		if (loaded_texture_desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			uint32_t color_count = 4; // B, G, R, A
			uint32_t array_size = color_count * texture_size.Width * texture_size.Height;
			unsigned char* data = new unsigned char[array_size];

			// Map the readable texture
			D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
			if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(readable_texture, 0, D3D11_MAP_READ, 0, &mapped_subresource)))
			{
				memcpy(data, mapped_subresource.pData, sizeof(unsigned char) * array_size);
				m_pDX->GetDeviceContext()->Unmap(readable_texture, 0);
			}

			uint32_t	converted_int{};
			float		converted_float{};
			float		v_x{};
			float		v_z{};
			float		v_y[4]{};

			// Vertex
			for (uint32_t z = 0; z < texture_size.Height - 1; ++z)
			{
				for (uint32_t x = 0; x < texture_size.Width - 1; ++x)
				{
					v_x = static_cast<float>(x);
					v_z = -static_cast<float>(z);

					// Ignore alpha value
					converted_int =
						data[(z * texture_size.Width * 4) + (x * 4)] +
						data[(z * texture_size.Width * 4) + (x * 4) + 1] * 256 +
						data[(z * texture_size.Width * 4) + (x * 4) + 2] * 65536;
					v_y[0] = static_cast<float>(converted_int) / HeightFactor;

					converted_int =
						data[(z * texture_size.Width * 4) + ((x + 1) * 4)] +
						data[(z * texture_size.Width * 4) + ((x + 1) * 4) + 1] * 256 +
						data[(z * texture_size.Width * 4) + ((x + 1) * 4) + 2] * 65536;
					v_y[1] = static_cast<float>(converted_int) / HeightFactor;

					converted_int =
						data[((z + 1) * texture_size.Width * 4) + (x * 4)] +
						data[((z + 1) * texture_size.Width * 4) + (x * 4) + 1] * 256 +
						data[((z + 1) * texture_size.Width * 4) + (x * 4) + 2] * 65536;
					v_y[2] = static_cast<float>(converted_int) / HeightFactor;

					converted_int =
						data[((z + 1) * texture_size.Width * 4) + ((x + 1) * 4)] +
						data[((z + 1) * texture_size.Width * 4) + ((x + 1) * 4) + 1] * 256 +
						data[((z + 1) * texture_size.Width * 4) + ((x + 1) * 4) + 2] * 65536;
					v_y[3] = static_cast<float>(converted_int) / HeightFactor;

					model_data.VertexData.AddVertex(SVertexModel(v_x, v_y[0], v_z, 0, 0));
					model_data.VertexData.AddVertex(SVertexModel(v_x + 1, v_y[1], v_z, 1, 0));
					model_data.VertexData.AddVertex(SVertexModel(v_x, v_y[2], v_z - 1, 0, 1));
					model_data.VertexData.AddVertex(SVertexModel(v_x + 1, v_y[3], v_z - 1, 1, 1));
				}
			}

			// Index
			for (uint32_t i = 0; i < (texture_size.Width - 1) * (texture_size.Height - 1); ++i)
			{
				model_data.IndexData.vIndices.push_back(SIndexTriangle(i * 4, i * 4 + 1, i * 4 + 2));
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
			float inv_uv_mat[2][2]{};
			XMFLOAT3 tangent{}, bitangent{};
			XMVECTOR normalized_tan{}, normalized_bitan{};
			
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
				inv_uv_mat[0][0] = det * dv_1;
				inv_uv_mat[0][1] = det * -dv_0;
				inv_uv_mat[1][0] = det * -du_1;
				inv_uv_mat[1][1] = det * du_0;
				
				tangent.x = XMVectorGetX(e_0) * inv_uv_mat[0][0] + XMVectorGetX(e_2) * inv_uv_mat[0][1];
				tangent.y = XMVectorGetY(e_0) * inv_uv_mat[0][0] + XMVectorGetY(e_2) * inv_uv_mat[0][1];
				tangent.z = XMVectorGetZ(e_0) * inv_uv_mat[0][0] + XMVectorGetZ(e_2) * inv_uv_mat[0][1];
				normalized_tan = XMLoadFloat3(&tangent);
				normalized_tan = XMVector3Normalize(normalized_tan);
				XMStoreFloat3(&tangent, normalized_tan);

				bitangent.x = XMVectorGetX(e_0) * inv_uv_mat[1][0] + XMVectorGetX(e_2) * inv_uv_mat[1][1];
				bitangent.y = XMVectorGetY(e_0) * inv_uv_mat[1][0] + XMVectorGetY(e_2) * inv_uv_mat[1][1];
				bitangent.z = XMVectorGetZ(e_0) * inv_uv_mat[1][0] + XMVectorGetZ(e_2) * inv_uv_mat[1][1];
				normalized_bitan = XMLoadFloat3(&bitangent);
				normalized_bitan = XMVector3Normalize(normalized_bitan);
				XMStoreFloat3(&bitangent, normalized_bitan);

				v_vertices[iter._0].Tangent = tangent;
				v_vertices[iter._1].Tangent = tangent;
				v_vertices[iter._2].Tangent = tangent;

				v_vertices[iter._0].Bitangent = bitangent;
				v_vertices[iter._1].Bitangent = bitangent;
				v_vertices[iter._2].Bitangent = bitangent;
			}
			
			JW_DELETE_ARRAY(data);
		}

		JW_RELEASE(readable_texture);
	}

	// Release all resources
	JW_RELEASE(texture);
	JW_RELEASE(texture_srv);

	return model_data;
}
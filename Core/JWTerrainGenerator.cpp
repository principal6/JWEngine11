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

					model_data.VertexData.AddVertex(SVertexModel(v_x	, v_y[0], v_z	 , 0, 0));
					model_data.VertexData.AddVertex(SVertexModel(v_x + 1, v_y[1], v_z	 , 1, 0));
					model_data.VertexData.AddVertex(SVertexModel(v_x	, v_y[2], v_z - 1, 0, 1));
					model_data.VertexData.AddVertex(SVertexModel(v_x + 1, v_y[3], v_z - 1, 1, 1));
				}
			}

			// Index
			for (uint32_t i = 0; i < (texture_size.Width - 1) * (texture_size.Height - 1); ++i)
			{
				model_data.IndexData.vIndices.push_back(SIndexTriangle(i * 4	, i * 4 + 1, i * 4 + 2));
				model_data.IndexData.vIndices.push_back(SIndexTriangle(i * 4 + 1, i * 4 + 3, i * 4 + 2));
			}

			// Compute normals
			auto& v_indices = model_data.IndexData.vIndices;
			auto& v_vertices = model_data.VertexData.vVerticesModel;
			XMVECTOR v_0{}, v_1{}, v_2{};
			XMVECTOR e_0{}, e_1{};
			XMVECTOR normal{};

			for (auto& iter : v_indices)
			{
				v_0 = XMLoadFloat3(&v_vertices[iter._0].Position);
				v_1 = XMLoadFloat3(&v_vertices[iter._1].Position);
				v_2 = XMLoadFloat3(&v_vertices[iter._2].Position);

				e_0 = v_1 - v_0;
				e_1 = v_2 - v_0;

				normal = XMVector3Cross(e_0, e_1);

				XMStoreFloat3(&v_vertices[iter._0].Normal, normal);
				XMStoreFloat3(&v_vertices[iter._1].Normal, normal);
				XMStoreFloat3(&v_vertices[iter._2].Normal, normal);
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
#pragma once

#include "JWEntity.h"

namespace JWEngine
{
	class JWDX;
	class JWCamera;
	
	enum class ESharedResourceType
	{
		Texture2D,
		TextureCubeMap,
	};

	enum class ESharedModelType
	{
		StaticModel,
		RiggedModel,
	};

	class JWECS final
	{
	public:
		JWECS() {};
		~JWECS()
		{
			if (m_vpEntities.size())
			{
				for (auto& iter : m_vpEntities)
				{
					JW_DELETE(iter);
				}

				for (auto& iter : m_vpSharedSRV)
				{
					JW_RELEASE(iter);
				}

				for (auto& iter : m_vAnimationTextureData)
				{
					JW_RELEASE(iter.Texture);
					JW_RELEASE(iter.TextureSRV);
				}

				for (auto& iter : m_vSharedModel)
				{
					iter.Destroy();
				}
			}
		};

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept
		{
			m_pDX = &DX;
			m_BaseDirectory = BaseDirectory;

			// @important
			m_SystemRender.CreateSystem(DX, Camera, BaseDirectory);
			m_SystemLight.CreateSystem(DX);
		}

		auto CreateEntity() noexcept->JWEntity&
		{
			m_vpEntities.push_back(new JWEntity(this));
			return *m_vpEntities[m_vpEntities.size() - 1];
		}

		auto GetEntity(uint32_t index) noexcept->JWEntity*
		{
			JWEntity* result{};

			if (index < m_vpEntities.size())
			{
				result = m_vpEntities[index];
			}

			return result;
		}

		void DestroyEntity(uint32_t index) noexcept
		{
			if (index < m_vpEntities.size())
			{
				JW_DELETE(m_vpEntities[index]);
				
				uint32_t last_index = static_cast<uint32_t>(m_vpEntities.size() - 1);
				if (index < last_index)
				{
					m_vpEntities[index] = m_vpEntities[last_index];
					m_vpEntities[last_index] = nullptr;
				}

				m_vpEntities.pop_back();
			}
		}

		void CreateSharedResource(ESharedResourceType Type, STRING FileName) noexcept
		{
			m_vpSharedSRV.push_back(nullptr);

			auto& current_srv = m_vpSharedSRV[m_vpSharedSRV.size() - 1];

			bool IsDDS{ false };
			if (FileName.find(".dds") != std::string::npos)
			{
				IsDDS = true;
			}
			WSTRING TextureFileName{};
			TextureFileName = StringToWstring(m_BaseDirectory + KAssetDirectory + FileName);

			switch (Type)
			{
			case JWEngine::ESharedResourceType::Texture2D:
				if (IsDDS)
				{
					CreateDDSTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &current_srv, 0);
				}
				else
				{
					CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &current_srv, 0);
				}
				break;
			case JWEngine::ESharedResourceType::TextureCubeMap:
				if (IsDDS)
				{
					CreateDDSTextureFromFileEx(m_pDX->GetDevice(), TextureFileName.c_str(), 0, D3D11_USAGE_DEFAULT,
						D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, false, nullptr,
						&current_srv);
				}
				break;
			default:
				break;
			}

			if (current_srv == nullptr)
			{
				m_vpSharedSRV.pop_back();
			}
		}

		void CreateSharedResourceFromSharedModel(size_t ModelIndex) noexcept
		{
			if (m_vSharedModel.size() == 0)
			{
				// No shared model exists.
				return;
			}

			ModelIndex = min(ModelIndex, m_vSharedModel.size() - 1);
			const JWModel* ptr_model = &m_vSharedModel[ModelIndex];

			m_vpSharedSRV.push_back(nullptr);

			auto& current_srv = m_vpSharedSRV[m_vpSharedSRV.size() - 1];

			bool IsDDS{ false };
			if (ptr_model->GetTextureFileName().find(L".dds") != WSTRING::npos)
			{
				IsDDS = true;
			}

			if (IsDDS)
			{
				CreateDDSTextureFromFile(m_pDX->GetDevice(), ptr_model->GetTextureFileName().c_str(), nullptr, &current_srv, 0);
			}
			else
			{
				CreateWICTextureFromFile(m_pDX->GetDevice(), ptr_model->GetTextureFileName().c_str(), nullptr, &current_srv, 0);
			}

			if (current_srv == nullptr)
			{
				m_vpSharedSRV.pop_back();
			}
		}

		auto GetSharedResource(size_t Index) noexcept->ID3D11ShaderResourceView*
		{
			ID3D11ShaderResourceView* result{};

			if (Index < m_vpSharedSRV.size())
			{
				result = m_vpSharedSRV[Index];
			}

			return result;
		}

		auto CreateSharedModelSquare(float Size, XMFLOAT2 UVMap) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);
			current_model.MakeSquare(Size, UVMap);

			return &current_model;
		}

		auto CreateSharedModelCircle(float Radius, uint8_t Detail) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);
			current_model.MakeCircle(Radius, Detail);

			return &current_model;
		}

		auto CreateSharedModelCube(float Size) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);
			current_model.MakeCube(Size);

			return &current_model;
		}

		auto CreateSharedModelPyramid(float Height, float Width) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);
			current_model.MakePyramid(Height, Width);

			return &current_model;
		}

		auto CreateSharedModelCone(float Height, float Radius, uint8_t Detail) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);
			current_model.MakeCone(Height, Radius, Detail);

			return &current_model;
		}

		auto CreateSharedModelCylinder(float Height, float Radius, uint8_t Detail) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);
			current_model.MakeCylinder(Height, Radius, Detail);

			return &current_model;
		}

		auto CreateSharedModelSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);
			current_model.MakeSphere(Radius, VerticalDetail, HorizontalDetail);

			return &current_model;
		}

		auto CreateSharedModelCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);
			current_model.MakeCapsule(Height, Radius, VerticalDetail, HorizontalDetail);

			return &current_model;
		}

		auto CreateSharedModelFromFile(ESharedModelType Type, STRING FileName) noexcept->JWModel*
		{
			m_vSharedModel.push_back(JWModel());

			auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

			current_model.Create(*m_pDX, m_BaseDirectory);

			JWAssimpLoader loader{};

			switch (Type)
			{
			case JWEngine::ESharedModelType::StaticModel:
				current_model.SetStaticModelData(loader.LoadStaticModel(m_BaseDirectory + KAssetDirectory, FileName));

				break;
			case JWEngine::ESharedModelType::RiggedModel:
				current_model.SetRiggedModelData(loader.LoadRiggedModel(m_BaseDirectory + KAssetDirectory, FileName));

				break;
			default:
				break;
			}

			return &current_model;
		}

		auto GetSharedModel(size_t Index) noexcept->JWModel*
		{
			JWModel* result{};

			if (Index < m_vSharedModel.size())
			{
				result = &m_vSharedModel[Index];
			}

			return result;
		}

		void CreateAnimationTexture(SSizeInt TextureSize) noexcept
		{
			m_vAnimationTextureData.push_back(SAnimationTextureData());

			auto& current_tex = m_vAnimationTextureData[m_vAnimationTextureData.size() - 1].Texture;
			auto& current_srv = m_vAnimationTextureData[m_vAnimationTextureData.size() - 1].TextureSRV;
			auto& current_tex_size = m_vAnimationTextureData[m_vAnimationTextureData.size() - 1].TextureSize;

			// Set texture format we want to use
			DXGI_FORMAT texture_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			
			// Describe the texture.
			D3D11_TEXTURE2D_DESC texture_descrption{};
			texture_descrption.Width = current_tex_size.Width = TextureSize.Width;
			texture_descrption.Height = current_tex_size.Height = TextureSize.Height;
			texture_descrption.MipLevels = 1;
			texture_descrption.ArraySize = 1;
			texture_descrption.Format = texture_format;
			texture_descrption.SampleDesc.Count = 1;
			texture_descrption.Usage = D3D11_USAGE_DYNAMIC;
			texture_descrption.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texture_descrption.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			texture_descrption.MiscFlags = 0;

			// Create the texture.
			m_pDX->GetDevice()->CreateTexture2D(&texture_descrption, nullptr, &current_tex);

			// Describe the shader resource view.
			D3D11_SHADER_RESOURCE_VIEW_DESC srv_description{};
			srv_description.Format = texture_format;
			srv_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_description.Texture2D.MostDetailedMip = 0;
			srv_description.Texture2D.MipLevels = 1;

			// Create the shader resource view.
			m_pDX->GetDevice()->CreateShaderResourceView(current_tex, &srv_description, &current_srv);

			if (current_srv == nullptr)
			{
				JW_RELEASE(current_tex);

				m_vAnimationTextureData.pop_back();
			}
		}

		void LoadAnimationTextureFromFile(STRING FileName) noexcept
		{
			m_vAnimationTextureData.push_back(SAnimationTextureData());

			auto& current_tex = m_vAnimationTextureData[m_vAnimationTextureData.size() - 1].Texture;
			auto& current_srv = m_vAnimationTextureData[m_vAnimationTextureData.size() - 1].TextureSRV;
			auto& current_tex_size = m_vAnimationTextureData[m_vAnimationTextureData.size() - 1].TextureSize;

			WSTRING w_fn = StringToWstring(m_BaseDirectory + KAssetDirectory + FileName);

			// Load texture from file.
			CreateDDSTextureFromFile(m_pDX->GetDevice(), w_fn.c_str(), (ID3D11Resource**)&current_tex, &current_srv, 0);

			// Get texture description from loaded texture.
			D3D11_TEXTURE2D_DESC loaded_texture_desc{};
			current_tex->GetDesc(&loaded_texture_desc);
			current_tex_size.Width = loaded_texture_desc.Width;
			current_tex_size.Height = loaded_texture_desc.Height;

			if (current_srv == nullptr)
			{
				JW_RELEASE(current_tex);

				m_vAnimationTextureData.pop_back();
			}
		}

		auto GetAnimationTexture(size_t Index) noexcept->SAnimationTextureData*
		{
			SAnimationTextureData* result{};

			if (Index < m_vAnimationTextureData.size())
			{
				result = &m_vAnimationTextureData[Index];
			}

			return result;
		}

		void UpdateAll() noexcept
		{
			m_SystemTransform.Update();

			m_SystemLight.Update();

			m_SystemRender.Update();
		}

		auto& SystemTransform() noexcept { return m_SystemTransform; }
		auto& SystemRender() noexcept { return m_SystemRender; }
		auto& SystemLight() noexcept { return m_SystemLight; }

	private:
		JWDX*				m_pDX{};
		STRING				m_BaseDirectory{};

		JWSystemTransform	m_SystemTransform{};
		JWSystemRender		m_SystemRender{};
		JWSystemLight		m_SystemLight{};

		VECTOR<JWEntity*>	m_vpEntities;

		// Shared resources(texture, model data, animation texture)
		VECTOR<ID3D11ShaderResourceView*>	m_vpSharedSRV;
		VECTOR<SAnimationTextureData>		m_vAnimationTextureData;
		VECTOR<JWModel>						m_vSharedModel;
	};
};


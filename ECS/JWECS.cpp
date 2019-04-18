#include "JWECS.h"
#include "../Core/JWWin32Window.h"

using namespace JWEngine;

void JWECS::Create(JWDX& DX, JWCamera& Camera, JWWin32Window& Window, STRING BaseDirectory) noexcept
{
	m_pDX = &DX;
	m_BaseDirectory = BaseDirectory;

	m_SystemTransform.Create();
	m_SystemRender.Create(*this, DX, Camera, BaseDirectory);
	m_SystemLight.Create(DX);
	m_SystemPhysics.Create(*this, Window, Camera);
}

void JWECS::Destroy() noexcept
{
	// Destroy entities and free them
	for (auto& iter : m_vpEntities)
	{
		iter->Destroy();
		JW_DELETE(iter);
	}

	// Destroy shared resources
	{
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

		for (auto& iter : m_vSharedLineModel)
		{
			iter.Destroy();
		}

		for (auto& iter : m_vSharedImage2D)
		{
			iter.Destroy();
		}
	}

	// Destroy systems
	m_SystemPhysics.Destroy();
	m_SystemLight.Destroy();
	m_SystemRender.Destroy();
	m_SystemTransform.Destroy();
}

auto JWECS::CreateEntity(STRING EntityName) noexcept->JWEntity*
{
	if (m_vpEntities.size())
	{
		for (auto& iter : m_vpEntities)
		{
			if (iter->GetEntityName().compare(EntityName) == 0)
			{
				// Duplicate name cannot be used!
				JW_ERROR_ABORT("This entity name is duplicated. (" + EntityName + ")");
			}
		}
	}

	m_vpEntities.push_back(new JWEntity());
	uint64_t index = m_vpEntities.size() - 1;

	auto& entity = m_vpEntities[index];
	entity->Create(this, EntityName);

	m_mapEntityNames.insert(std::make_pair(EntityName, index));

	return entity;
}

auto JWECS::CreateEntity(EEntityType Type) noexcept->JWEntity*
{
	if (Type == EEntityType::UserDefined)
	{
		JW_ERROR_ABORT("You can't make a user-defined entity by this method.");
	}

	if (m_vpEntities.size())
	{
		for (auto& iter : m_vpEntities)
		{
			if (iter->GetEntityType() == Type)
			{
				// It must be unique
				JW_ERROR_ABORT("The entity of the type already exists.");
			}
		}
	}

	STRING temp{};
	STRING entity_name{ "UniqueEntity" };
	uint32_t type_id = static_cast<uint32_t>(Type);
	entity_name += ConvertIntToSTRING(type_id, temp);

	m_vpEntities.push_back(new JWEntity());
	uint64_t index = m_vpEntities.size() - 1;

	auto& entity = m_vpEntities[index];
	entity->Create(this, entity_name, Type);
	m_mapEntityNames.insert(std::make_pair(entity_name, index));

	return entity;
}

auto JWECS::GetEntity(uint32_t index) noexcept->JWEntity*
{
	JWEntity* result{};

	if (index < m_vpEntities.size())
	{
		result = m_vpEntities[index];
	}

	return result;
}

auto JWECS::GetEntityByName(STRING EntityName) noexcept->JWEntity*
{
	JWEntity* result{};

	if (m_vpEntities.size())
	{
		auto find = m_mapEntityNames.find(EntityName);
		if (find != m_mapEntityNames.end())
		{
			auto index = find->second;

			result = m_vpEntities[index];
		}
		else
		{
			JW_ERROR_ABORT("Unable to fine the entity of name (" + EntityName + ")");
		}
	}

	return result;
}

auto JWECS::GetEntityByType(EEntityType Type) noexcept->JWEntity*
{
	JWEntity* result{};

	if (Type == EEntityType::UserDefined)
	{
		JW_ERROR_ABORT("Impossible to get the entity of user defined type by calling this method.");
	}

	if (m_vpEntities.size())
	{
		for (auto& iter : m_vpEntities)
		{
			if (iter->GetEntityType() == Type)
			{
				result = iter;
			}
		}
	}

	return result;
}

void JWECS::DestroyEntity(uint32_t index) noexcept
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

void JWECS::CreateSharedTexture(ESharedTextureType Type, STRING FileName) noexcept
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
	case JWEngine::ESharedTextureType::Texture2D:
		if (IsDDS)
		{
			CreateDDSTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &current_srv, 0);
		}
		else
		{
			CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &current_srv, 0);
		}
		break;
	case JWEngine::ESharedTextureType::TextureCubeMap:
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

void JWECS::CreateSharedTextureFromSharedModel(size_t ModelIndex) noexcept
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

auto JWECS::GetSharedTexture(size_t Index) noexcept->ID3D11ShaderResourceView*
{
	ID3D11ShaderResourceView* result{};

	if (Index < m_vpSharedSRV.size())
	{
		result = m_vpSharedSRV[Index];
	}

	return result;
}

auto JWECS::CreateSharedModelPrimitive(const SModelData& ModelData) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.CreateMeshBuffers(ModelData, ERenderType::Model_Static);

	return &current_model;
}

auto JWECS::CreateSharedModelDynamicPrimitive(const SModelData& ModelData) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.CreateMeshBuffers(ModelData, ERenderType::Model_Dynamic);

	return &current_model;
}

auto JWECS::CreateSharedModelFromFile(ESharedModelType Type, STRING FileName) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWAssimpLoader loader{};

	switch (Type)
	{
	case JWEngine::ESharedModelType::StaticModel:
		current_model.CreateMeshBuffers(loader.LoadNonRiggedModel(m_BaseDirectory + KAssetDirectory, FileName), ERenderType::Model_Static);

		break;
	case JWEngine::ESharedModelType::RiggedModel:
		current_model.CreateMeshBuffers(loader.LoadRiggedModel(m_BaseDirectory + KAssetDirectory, FileName), ERenderType::Model_Rigged);

		break;
	default:
		break;
	}

	return &current_model;
}

auto JWECS::GetSharedModel(size_t Index) noexcept->JWModel*
{
	JWModel* result{};

	if (Index < m_vSharedModel.size())
	{
		result = &m_vSharedModel[Index];
	}

	return result;
}

auto JWECS::CreateSharedLineModel() noexcept->JWLineModel*
{
	m_vSharedLineModel.push_back(JWLineModel());

	auto& current_line_model = m_vSharedLineModel[m_vSharedLineModel.size() - 1];

	current_line_model.Create(*m_pDX);

	return &current_line_model;
}

auto JWECS::GetSharedLineModel(size_t Index) noexcept->JWLineModel*
{
	JWLineModel* result{};

	if (Index < m_vSharedLineModel.size())
	{
		result = &m_vSharedLineModel[Index];
	}

	return result;
}

auto JWECS::CreateSharedImage2D(SPositionInt Position, SSizeInt Size) noexcept->JWImage*
{
	m_vSharedImage2D.push_back(JWImage());

	auto& current_image = m_vSharedImage2D[m_vSharedImage2D.size() - 1];

	current_image.Create(*m_pDX);

	current_image.SetPosition(XMFLOAT2(static_cast<float>(Position.X), static_cast<float>(Position.Y)));
	current_image.SetSize(XMFLOAT2(static_cast<float>(Size.Width), static_cast<float>(Size.Height)));

	return &current_image;
}

auto JWECS::GetSharedImage2D(size_t Index) noexcept->JWImage*
{
	JWImage* result{};

	if (Index < m_vSharedImage2D.size())
	{
		result = &m_vSharedImage2D[Index];
	}

	return result;
}

void JWECS::CreateAnimationTextureFromFile(STRING FileName) noexcept
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

auto JWECS::GetAnimationTexture(size_t Index) noexcept->SAnimationTextureData*
{
	SAnimationTextureData* result{};

	if (Index < m_vAnimationTextureData.size())
	{
		result = &m_vAnimationTextureData[Index];
	}

	return result;
}

void JWECS::ExecuteSystems() noexcept
{
	m_SystemTransform.Execute();

	m_SystemLight.Execute();

	m_SystemRender.Execute();

	m_SystemPhysics.Execute();
}
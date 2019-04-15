#include "JWECS.h"

using namespace JWEngine;

void JWECS::Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept
{
	m_pDX = &DX;
	m_BaseDirectory = BaseDirectory;

	// @important
	m_SystemRender.CreateSystem(DX, Camera, BaseDirectory);
	m_SystemLight.CreateSystem(DX);
}

void JWECS::Destroy() noexcept
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

		for (auto& iter : m_vSharedLineModel)
		{
			iter.Destroy();
		}

		for (auto& iter : m_vSharedImage2D)
		{
			iter.Destroy();
		}
	}
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
				MessageBoxA(nullptr, "엔티티 이름이 중복되었습니다. 다른 이름을 써 주세요.", "오류", MB_OK);

				return nullptr;
			}
		}
	}

	m_vpEntities.push_back(new JWEntity(this, EntityName));
	
	uint64_t index = m_vpEntities.size() - 1;
	m_mapEntityNames.insert(std::make_pair(EntityName, index));

	return m_vpEntities[index];
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

	assert(m_vpEntities.size());

	auto find = m_mapEntityNames.find(EntityName);
	if (find != m_mapEntityNames.end())
	{
		auto index = find->second;

		result = m_vpEntities[index];
	}
	else
	{
		MessageBoxA(nullptr, "엔티티 이름이 잘못되었습니다.", "오류", MB_OK);
	}

	return result;
}

auto JWECS::GetUniqueEntity(EEntityType Type) noexcept->JWEntity*
{
	JWEntity* result{};

	assert(m_vpEntities.size());
	
	if (JW_IS_UNIQUE_ENTITY_TYPE(Type))
	{
		uint32_t index_of_type = static_cast<uint32_t>(Type);

		result = m_pUniqueEntities[index_of_type];
	}

	return result;
}

void JWECS::SetUniqueEntity(JWEntity* PtrEntity, EEntityType Type) noexcept
{
	assert(JW_IS_UNIQUE_ENTITY_TYPE(Type));

	uint32_t index_of_type = static_cast<uint32_t>(Type);

	// Avoid duplicate setting! It must be UNIQUE!
	if (m_pUniqueEntities[index_of_type] == nullptr)
	{
		m_pUniqueEntities[index_of_type] = PtrEntity;
	}
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

void JWECS::PickEntityTriangle(XMVECTOR& RayOrigin, XMVECTOR& RayDirection) noexcept
{
	if (m_vpEntities.size())
	{
		XMVECTOR t_cmp{ XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 0) };

		for (auto iter : m_vpEntities)
		{
			auto transform{ iter->GetComponentTransform() };
			auto render{ iter->GetComponentRender() };
			auto type{ iter->GetEntityType() };
			
			if ((type == EEntityType::Sky) || (type == EEntityType::Grid) ||
				(type == EEntityType::PickingRay) || (type == EEntityType::PickedTriangle))
			{
				continue;
			}

			if (transform)
			{
				auto model_type = render->PtrModel->GetRenderType();
				if ((model_type == ERenderType::Model_Dynamic) || (model_type == ERenderType::Model_Static))
				{
					auto indices = render->PtrModel->NonRiggedModelData.IndexData.vIndices;
					auto vertices = render->PtrModel->NonRiggedModelData.VertexData.vVertices;

					// Iterate all the triangles in the model
					for (auto triangle : indices)
					{
						auto v0 = XMLoadFloat3(&vertices[triangle._0].Position);
						auto v1 = XMLoadFloat3(&vertices[triangle._1].Position);
						auto v2 = XMLoadFloat3(&vertices[triangle._2].Position);

						// Move vertices from local space to world space!
						v0 = XMVector3TransformCoord(v0, transform->WorldMatrix);
						v1 = XMVector3TransformCoord(v1, transform->WorldMatrix);
						v2 = XMVector3TransformCoord(v2, transform->WorldMatrix);

						auto t = PickTriangle(v0, v1, v2, RayOrigin, RayDirection, t_cmp);
					}
				}
				else if (model_type == ERenderType::Model_Rigged)
				{
					auto indices = render->PtrModel->RiggedModelData.IndexData.vIndices;
					auto vertices = render->PtrModel->RiggedModelData.VertexData.vVertices;

					// Iterate all the triangles in the model
					for (auto triangle : indices)
					{
						auto v0 = XMLoadFloat3(&vertices[triangle._0].Position);
						auto v1 = XMLoadFloat3(&vertices[triangle._1].Position);
						auto v2 = XMLoadFloat3(&vertices[triangle._2].Position);

						// Move vertices from local space to world space!
						v0 = XMVector3TransformCoord(v0, transform->WorldMatrix);
						v1 = XMVector3TransformCoord(v1, transform->WorldMatrix);
						v2 = XMVector3TransformCoord(v2, transform->WorldMatrix);

						auto t = PickTriangle(v0, v1, v2, RayOrigin, RayDirection, t_cmp);
					}
				}
			}
		}
	}
}

PRIVATE auto JWECS::PickTriangle(XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2,
	XMVECTOR& RayOrigin, XMVECTOR& RayDirection, XMVECTOR& t_cmp) noexcept->XMVECTOR
{
	// Calculate edge vectors from vertex positions
	auto edge0 = V1 - V0;
	auto edge1 = V2 - V0;

	// Calculate face normal from edge vectors,
	// using cross product of vectors
	auto normal = XMVector3Normalize(XMVector3Cross(edge0, edge1));

	// Calculate plane equation  # Ax + By + Cz + D = 0
	// # A, B, C = Face normal's xyz coord
	// # x, y, z = Any point in the plane, so we can just use V0
	// # Ax + By + Cz = Dot(normal, point) = Dot(N, P)
	// # D = -(Ax + By + Cz) = -Dot(normal, v0) = -Dot(N, V0)
	//
	// @ Plane equation for a point P
	// Dot(N, P) = Dot(N, V0)
	auto D = -XMVector3Dot(normal, V0);

	// Get ray's equation (which is a parametric equation of a line)
	//
	// Line = P0 + t * P1
	//      = ray_origin + t * ray_direction
	//
	// @ N: plane normal  @ V = given vertex in the plane  @ L = line vector
	//
	// L = (P0x + P1x * t, P0y + P1y * t, P0z + P1z * t)
	//
	// Dot(N, L) = Dot(N, V)
	// => (P0x + P1x * t) * Nx + (P0y + P1y * t) * Ny + (P0z + P1z * t) * Nz = (Vx) * Nx + (Vy) * Ny + (Vz) * Nz
	// => (P1x * t) * Nx + (P1y * t) * Ny + (P1z * t) * Nz = (Vx - P0x) * Nx + (Vy - P0y) * Ny + (Vz - P0z) * Nz
	// => (P1x * Nx) * t + (P1y * Ny) * t + (P1z * Nz) * t = (Vx - P0x) * Nx + (Vy - P0y) * Ny + (Vz - P0z) * Nz
	// => Dot(P1, N) * t = Dot(V, N) - Dot(P0, N)
	//
	//           Dot(V, N) - Dot(P0, N)
	// =>  t  = ------------------------
	//                 Dot(P1, N)
	//
	auto p0_norm = XMVector3Dot(RayOrigin, normal);
	auto p1_norm = XMVector3Dot(RayDirection, normal);
	XMVECTOR zero{ XMVectorZero() };
	XMVECTOR t{};

	// For vectorization we use vectors to compare values
	if (XMVector3NotEqual(p1_norm, zero))
	{
		t = (-D - p0_norm) / p1_norm;
	}

	// 't' should be positive for the picking to be in front of the camera!
	// (if it's negative, the picking is occuring behind the camera)
	// We will store the minimum of t values, which means that it's the closest picking to the camera.
	if ((XMVector3Greater(t, zero)) && (XMVector3Less(t, t_cmp)))
	{
		auto point_on_plane = RayOrigin + t * RayDirection;

		// Check if the point is in the triangle
		if (IsPointInTriangle(point_on_plane, V0, V1, V2))
		{
			m_PickedTriangle[0] = V0;
			m_PickedTriangle[1] = V1;
			m_PickedTriangle[2] = V2;

			t_cmp = t;
		}
	}

	return t;
}

PRIVATE auto JWECS::IsPointInTriangle(XMVECTOR& Point, XMVECTOR& V0, XMVECTOR& V1, XMVECTOR& V2) noexcept->bool
{
	bool result{ false };
	auto zero = XMVectorZero();
	auto check_0 = XMVector3Cross((V2 - V1), (Point - V1));
	auto check_1 = XMVector3Cross((V2 - V1), (V0 - V1));

	if (XMVector3Greater(XMVector3Dot(check_0, check_1), zero))
	{
		check_0 = XMVector3Cross((V2 - V0), (Point - V0));
		check_1 = XMVector3Cross((V2 - V0), (V1 - V0));

		if (XMVector3Greater(XMVector3Dot(check_0, check_1), zero))
		{
			check_0 = XMVector3Cross((V1 - V0), (Point - V0));
			check_1 = XMVector3Cross((V1 - V0), (V2 - V0));

			if (XMVector3Greater(XMVector3Dot(check_0, check_1), zero))
			{
				// In triangle!
				result = true;
			}
		}
	}

	return result;
}

auto JWECS::GetPickedTrianglePosition(uint32_t PositionIndex) const noexcept->const XMVECTOR&
{
	PositionIndex = min(PositionIndex, 2);

	return m_PickedTriangle[PositionIndex];
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

auto JWECS::CreateSharedModelPrimitive(const SNonRiggedModelData& ModelData) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(ModelData);

	return &current_model;
}

auto JWECS::CreateSharedModelDynamicPrimitive(const SNonRiggedModelData& ModelData) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetDynamicModelData(ModelData);

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
		current_model.SetNonRiggedModelData(loader.LoadNonRiggedModel(m_BaseDirectory + KAssetDirectory, FileName));

		break;
	case JWEngine::ESharedModelType::RiggedModel:
		current_model.SetRiggedModelData(loader.LoadRiggedModel(m_BaseDirectory + KAssetDirectory, FileName));

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
}
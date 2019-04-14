#include "JWECS.h"

using namespace JWEngine;

JWECS::~JWECS()
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

void JWECS::Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept
{
	m_pDX = &DX;
	m_BaseDirectory = BaseDirectory;

	// @important
	m_SystemRender.CreateSystem(DX, Camera, BaseDirectory);
	m_SystemLight.CreateSystem(DX);
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

						// Calculate edge vectors from positions
						auto edge0 = v1 - v0;
						auto edge1 = v2 - v0;

						// Calculate face normal from edge vectors,
						// using cross product of vectors
						auto normal = XMVector3Normalize(XMVector3Cross(edge0, edge1));

						// Calculate plane equation  # Ax + By + Cz + D = 0
						// # A, B, C = Face normal's xyz coord
						// # x, y, z = Any point in the plane, so we use v0 
						// # D = -(Ax + By + Cz) = -Dot(normal, v0)
						auto D = -XMVector3Dot(normal, v0);

						// Get ray's equation (which is a parametric equation of a line)
						// Line = P_0 + tP_1 (t = [0, 1])
						//      = ray_origin + t * ray_direction (t = [0, 1])
						auto p_0 = XMVector3Dot(RayOrigin, normal);
						auto p_1 = XMVector3Dot(RayDirection, normal);
						XMVECTOR t{};

						// for vectorization...
						XMVECTOR zero = XMVectorZero();
						if (XMVector3NotEqual(p_1, zero))
						{
							t = -(p_0 + D) / p_1;
						}

						// 't' should be positive for the picking to be in front of the camera!
						// and we will store the minimum t, which is the closest to the camera.
						if ((XMVector3Greater(t, zero)) && (XMVector3Less(t, t_cmp)))
						{
							auto point_on_plane = RayOrigin + t * RayDirection;

							// Check if the point is in the triangle
							if (IsPointInTriangle(point_on_plane, v0, v1, v2))
							{
								m_PickedTriangle[0] = v0;
								m_PickedTriangle[1] = v1;
								m_PickedTriangle[2] = v2;

								t_cmp = t;
							}
						}
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

						// Calculate edge vectors from positions
						auto edge0 = v1 - v0;
						auto edge1 = v2 - v0;

						// Calculate face normal from edge vectors,
						// using cross product of vectors
						auto normal = XMVector3Normalize(XMVector3Cross(edge0, edge1));

						// Calculate plane equation  # Ax + By + Cz + D = 0
						// # A, B, C = Face normal's xyz coord
						// # x, y, z = Any point in the plane, so we use v0 
						// # D = -(Ax + By + Cz) = -Dot(normal, v0)
						auto D = -XMVector3Dot(normal, v0);

						// Get ray's equation (which is a parametric equation of a line)
						// Line = P_0 + tP_1 (t = [0, 1])
						//      = ray_origin + t * ray_direction (t = [0, 1])
						auto p_0 = XMVector3Dot(RayOrigin, normal);
						auto p_1 = XMVector3Dot(RayDirection, normal);
						XMVECTOR t{};

						// for vectorization...
						XMVECTOR zero = XMVectorZero();
						if (XMVector3NotEqual(p_1, zero))
						{
							t = -(p_0 + D) / p_1;
						}

						// 't' should be positive for the picking to be in front of the camera!
						// and we will store the minimum t, which is the closest to the camera.
						if ((XMVector3Greater(t, zero)) && (XMVector3Less(t, t_cmp)))
						{
							auto point_on_plane = RayOrigin + t * RayDirection;

							// Check if the point is in the triangle
							if (IsPointInTriangle(point_on_plane, v0, v1, v2))
							{
								m_PickedTriangle[0] = v0;
								m_PickedTriangle[1] = v1;
								m_PickedTriangle[2] = v2;

								t_cmp = t;
							}
						}
					}
				}
			}
		}
	}
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

void JWECS::CreateSharedResource(ESharedResourceType Type, STRING FileName) noexcept
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

void JWECS::CreateSharedResourceFromSharedModel(size_t ModelIndex) noexcept
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

auto JWECS::GetSharedResource(size_t Index) noexcept->ID3D11ShaderResourceView*
{
	ID3D11ShaderResourceView* result{};

	if (Index < m_vpSharedSRV.size())
	{
		result = m_vpSharedSRV[Index];
	}

	return result;
}

auto JWECS::CreateSharedModelTriangle(XMFLOAT3 A, XMFLOAT3 B, XMFLOAT3 C, bool IsDynamicModel) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	if (IsDynamicModel)
	{
		current_model.SetDynamicModelData(maker.MakeTriangle(A, B, C));
	}
	else
	{
		current_model.SetNonRiggedModelData(maker.MakeTriangle(A, B, C));
	}

	return &current_model;
}

auto JWECS::CreateSharedModelSquare(float Size, XMFLOAT2 UVMap) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(maker.MakeSquare(Size, UVMap));

	return &current_model;
}

auto JWECS::CreateSharedModelCircle(float Radius, uint8_t Detail) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(maker.MakeCircle(Radius, Detail));

	return &current_model;
}

auto JWECS::CreateSharedModelCube(float Size) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(maker.MakeCube(Size));

	return &current_model;
}

auto JWECS::CreateSharedModelPyramid(float Height, float Width) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(maker.MakePyramid(Height, Width));

	return &current_model;
}

auto JWECS::CreateSharedModelCone(float Height, float Radius, uint8_t Detail) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(maker.MakeCone(Height, Radius, Detail));

	return &current_model;
}

auto JWECS::CreateSharedModelCylinder(float Height, float Radius, uint8_t Detail) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(maker.MakeCylinder(Height, Radius, Detail));

	return &current_model;
}

auto JWECS::CreateSharedModelSphere(float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(maker.MakeSphere(Radius, VerticalDetail, HorizontalDetail));

	return &current_model;
}

auto JWECS::CreateSharedModelCapsule(float Height, float Radius, uint8_t VerticalDetail, uint8_t HorizontalDetail) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.SetNonRiggedModelData(maker.MakeCapsule(Height, Radius, VerticalDetail, HorizontalDetail));
	
	return &current_model;
}

auto JWECS::CreateSharedModelFromFile(ESharedModelType Type, STRING FileName) noexcept->JWECS&
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

	return *this;
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

auto JWECS::AddAnimationToModelFromFile(size_t Index, STRING FileName) noexcept->JWECS&
{
	JWModel* model{};

	if (model = GetSharedModel(Index))
	{
		auto type = model->GetRenderType();

		if (type == ERenderType::Model_Rigged)
		{
			JWAssimpLoader loader{};
			loader.LoadAdditionalAnimationIntoRiggedModel(model->RiggedModelData, m_BaseDirectory + KAssetDirectory, FileName);
		}
	}

	return *this;
}

auto JWECS::BakeAnimationTextureToFile(size_t ModelIndex, SSizeInt TextureSize, STRING FileName) noexcept->JWECS&
{
	JWModel* model{};

	if (model = GetSharedModel(ModelIndex))
	{
		if (model->RiggedModelData.AnimationSet.vAnimations.size())
		{
			SAnimationTextureData texture_data{};
			auto& current_tex = texture_data.Texture;
			auto& current_srv = texture_data.TextureSRV;
			auto& current_tex_size = texture_data.TextureSize;

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

			assert(current_tex);

			// Describe the shader resource view.
			D3D11_SHADER_RESOURCE_VIEW_DESC srv_description{};
			srv_description.Format = texture_format;
			srv_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_description.Texture2D.MostDetailedMip = 0;
			srv_description.Texture2D.MipLevels = 1;

			// Create the shader resource view.
			m_pDX->GetDevice()->CreateShaderResourceView(current_tex, &srv_description, &current_srv);

			auto& vec_animations{ model->RiggedModelData.AnimationSet.vAnimations };

			uint32_t texel_count{ TextureSize.Width * TextureSize.Height };
			uint32_t texel_y_advance{ TextureSize.Width * KColorCountPerTexel };
			uint32_t data_size{ texel_count * KColorCountPerTexel };
			float* data = new float[data_size] {};

			//
			// Set animation set's info
			// (with maximum bone count = KMaxBoneCount)
			// data[0 ~ 3] = Animation ID 0 = TPose
			//
			// TPose frame count(=texel line count)
			data[0] = 1;
			// TPose texel start y index
			data[1] = 1;
			// Reserved
			data[2] = 0;
			// Reserved
			data[3] = 0;

			for (uint16_t iter_anim = 0; iter_anim < vec_animations.size(); iter_anim++)
			{
				// current animation's frame count(=texel line count)
				data[4 + iter_anim * 4] = static_cast<float>(vec_animations[iter_anim].TotalFrameCount);

				// current animation's texel start y index
				data[4 + iter_anim * 4 + 1] = data[iter_anim * 4] + data[1 + iter_anim * 4];

				// Reserved
				//data[4 + iter_anim * 4 + 2] = 0;

				// Reserved
				//data[4 + iter_anim * 4 + 3] = 0;
			}

			// Bake animations into the texture
			uint32_t start_index{ texel_y_advance };
			XMMATRIX frame_matrices[KMaxBoneCount]{};

			// TPose into FrameMatrices
			SaveTPoseIntoFrameMatrices(frame_matrices, model->RiggedModelData,
				model->RiggedModelData.NodeTree.vNodes[0], XMMatrixIdentity());

			// Bake FrameMatrices into Texture
			BakeCurrentFrameIntoTexture(start_index, frame_matrices, data);

			// Update start index
			start_index += texel_y_advance;

			for (uint16_t anim_index = 0; anim_index < vec_animations.size(); anim_index++)
			{
				// This loop iterates each animation, starting from vec_animations[0]

				float frame_time{};

				for (uint16_t frame_index = 0; frame_index < vec_animations[anim_index].TotalFrameCount; frame_index++)
				{
					frame_time = static_cast<float>(frame_index) * vec_animations[anim_index].AnimationTicksPerGameTick;

					// Current frame into FrameMatrices
					SaveAnimationFrameIntoFrameMatrices(frame_matrices, anim_index, frame_time,
						model->RiggedModelData, model->RiggedModelData.NodeTree.vNodes[0], XMMatrixIdentity());

					// Bake FrameMatrices into Texture
					BakeCurrentFrameIntoTexture(start_index, frame_matrices, data);

					// Update start index
					start_index += texel_y_advance;
				}
			}

			// Map data into texture
			D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
			if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(current_tex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
			{
				memcpy(mapped_subresource.pData, data, sizeof(float) * data_size);

				m_pDX->GetDeviceContext()->Unmap(current_tex, 0);
			}

			JW_DELETE_ARRAY(data);

			// Save texture into file
			WSTRING w_fn = StringToWstring(m_BaseDirectory + KAssetDirectory + FileName);
			SaveDDSTextureToFile(m_pDX->GetDeviceContext(), current_tex, w_fn.c_str());

			JW_RELEASE(current_tex);
			JW_RELEASE(current_srv);
		}

	}

	return *this;
}

PRIVATE void JWECS::SaveTPoseIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const SRiggedModelData& ModelData,
	const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept
{
	XMMATRIX accumulation = CurrentNode.Transformation * Accumulated;

	if (CurrentNode.BoneID >= 0)
	{
		auto& bone = ModelData.BoneTree.vBones[CurrentNode.BoneID];

		OutFrameMatrices[CurrentNode.BoneID] = bone.Offset * accumulation;
	}

	if (CurrentNode.vChildrenID.size())
	{
		for (auto child_id : CurrentNode.vChildrenID)
		{
			SaveTPoseIntoFrameMatrices(OutFrameMatrices, ModelData, ModelData.NodeTree.vNodes[child_id], accumulation);
		}
	}
}

PRIVATE void JWECS::SaveAnimationFrameIntoFrameMatrices(XMMATRIX* OutFrameMatrices, const int AnimationID, const float FrameTime,
	const SRiggedModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept
{
	XMMATRIX global_transformation = CurrentNode.Transformation * Accumulated;

	if (CurrentNode.BoneID >= 0)
	{
		auto& bone = ModelData.BoneTree.vBones[CurrentNode.BoneID];
		auto& current_animation = ModelData.AnimationSet.vAnimations[AnimationID];

		for (const auto& CurrentNode_animation : current_animation.vNodeAnimation)
		{
			if (CurrentNode_animation.NodeID == CurrentNode.ID)
			{
				XMVECTOR scaling_key_a{};
				XMVECTOR rotation_key_a{};
				XMVECTOR translation_key_a{};

				// #1. Find scaling keys
				for (const auto& key : CurrentNode_animation.vKeyScaling)
				{
					if (key.TimeInTicks <= FrameTime)
					{
						scaling_key_a = XMLoadFloat3(&key.Key);
					}
				}

				// #2. Find rotation keys
				for (const auto& key : CurrentNode_animation.vKeyRotation)
				{
					if (key.TimeInTicks <= FrameTime)
					{
						rotation_key_a = key.Key;
					}
				}

				// #3. Find translation keys
				for (const auto& key : CurrentNode_animation.vKeyPosition)
				{
					if (key.TimeInTicks <= FrameTime)
					{
						translation_key_a = XMLoadFloat3(&key.Key);
					}
				}

				XMMATRIX matrix_scaling{ XMMatrixScalingFromVector(scaling_key_a) };
				XMMATRIX matrix_rotation{ XMMatrixRotationQuaternion(rotation_key_a) };
				XMMATRIX matrix_translation{ XMMatrixTranslationFromVector(translation_key_a) };

				global_transformation = matrix_scaling * matrix_rotation * matrix_translation * Accumulated;

				break;
			}
		}

		OutFrameMatrices[CurrentNode.BoneID] = bone.Offset * global_transformation;
	}

	if (CurrentNode.vChildrenID.size())
	{
		for (auto child_id : CurrentNode.vChildrenID)
		{
			SaveAnimationFrameIntoFrameMatrices(OutFrameMatrices, AnimationID, FrameTime,
				ModelData, ModelData.NodeTree.vNodes[child_id], global_transformation);
		}
	}
}

PRIVATE void JWECS::BakeCurrentFrameIntoTexture(uint32_t StartIndex, const XMMATRIX* FrameMatrices, float*& OutData) noexcept
{
	XMFLOAT4X4A current_matrix{};
	const int matrix_size_in_floats = 16;

	for (uint16_t bone_index = 0; bone_index < KMaxBoneCount; ++bone_index)
	{
		XMStoreFloat4x4(&current_matrix, FrameMatrices[bone_index]);

		memcpy(&OutData[StartIndex + bone_index * matrix_size_in_floats], current_matrix.m, sizeof(float) * matrix_size_in_floats);
	}
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
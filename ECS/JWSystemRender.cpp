#include "JWECS.h"
#include "../Core/JWAssimpLoader.h"

using namespace JWEngine;

void JWSystemRender::Create(JWECS& ECS, JWDX& DX, STRING BaseDirectory) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set base directory.
	m_BaseDirectory = BaseDirectory;

	// Bounding volume (with instance buffer)
	JWPrimitiveMaker primitive{};
	m_BoundingVolume.Create(DX, BaseDirectory);
	m_BoundingVolume.CreateMeshBuffers(primitive.MakeSphere(1.0f, 16, 7), ERenderType::Model_Static);
	m_BoundingVolume.CreateInstanceBuffer();

	// Terrain
	m_TerrainGenerator.Create(DX, BaseDirectory);
}

void JWSystemRender::Destroy() noexcept
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}

	// Destroy shared resources
	{
		for (auto& iter : m_vSharedTextureData)
		{
			JW_RELEASE(iter.Texture);
			JW_RELEASE(iter.TextureSRV);
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

		for (auto& iter : m_vSharedTerrain)
		{
			iter.Destroy();
		}
	}

	m_TerrainGenerator.Destroy();

	m_BoundingVolume.Destroy();
}

auto JWSystemRender::CreateComponent(JWEntity* pEntity) noexcept->SComponentRender&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };

	auto new_entry{ new SComponentRender() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID & pointer to Entity
	m_vpComponents[slot]->ComponentID = slot;
	m_vpComponents[slot]->PtrEntity = pEntity;

	return *m_vpComponents[slot];
}

void JWSystemRender::DestroyComponent(SComponentRender& Component) noexcept
{
	uint32_t slot{};
	for (const auto& iter : m_vpComponents)
	{
		if (iter->ComponentID == Component.ComponentID)
		{
			break;
		}

		++slot;
	}
	JW_DELETE(m_vpComponents[slot]);

	// Swap the last element of the vector and the deleted element & shrink the size of the vector.
	uint32_t last_index = static_cast<uint32_t>(m_vpComponents.size() - 1);
	if (slot < last_index)
	{
		m_vpComponents[slot] = m_vpComponents[last_index];
		m_vpComponents[slot]->ComponentID = slot; // @important

		m_vpComponents[last_index] = nullptr;
	}

	m_vpComponents.pop_back();
}

void JWSystemRender::CreateSharedTexture(ESharedTextureType Type, STRING FileName) noexcept
{
	m_vSharedTextureData.push_back(STextureData());

	auto& texture = m_vSharedTextureData[m_vSharedTextureData.size() - 1].Texture;
	auto& texture_srv = m_vSharedTextureData[m_vSharedTextureData.size() - 1].TextureSRV;
	auto& texture_size = m_vSharedTextureData[m_vSharedTextureData.size() - 1].TextureSize;

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
			CreateDDSTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &texture_srv, 0);
		}
		else
		{
			CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &texture_srv, 0);
		}
		break;
	case JWEngine::ESharedTextureType::TextureCubeMap:
		if (IsDDS)
		{
			CreateDDSTextureFromFileEx(m_pDX->GetDevice(), TextureFileName.c_str(), 0, D3D11_USAGE_DEFAULT,
				D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, false,
				nullptr, &texture_srv);
		}
		break;
	default:
		break;
	}

	if (texture_srv == nullptr)
	{
		JW_RELEASE(texture);

		m_vSharedTextureData.pop_back();
	}
}

void JWSystemRender::CreateSharedTextureFromSharedModel(size_t ModelIndex) noexcept
{
	if (m_vSharedModel.size() == 0)
	{
		// No shared model exists.
		return;
	}

	ModelIndex = min(ModelIndex, m_vSharedModel.size() - 1);
	const JWModel * ptr_model = &m_vSharedModel[ModelIndex];

	m_vSharedTextureData.push_back(STextureData());

	auto& texture = m_vSharedTextureData[m_vSharedTextureData.size() - 1].Texture;
	auto& texture_srv = m_vSharedTextureData[m_vSharedTextureData.size() - 1].TextureSRV;
	auto& texture_size = m_vSharedTextureData[m_vSharedTextureData.size() - 1].TextureSize;

	bool IsDDS{ false };
	if (ptr_model->GetTextureFileName().find(L".dds") != WSTRING::npos)
	{
		IsDDS = true;
	}

	if (IsDDS)
	{
		CreateDDSTextureFromFile(m_pDX->GetDevice(), ptr_model->GetTextureFileName().c_str(), nullptr, &texture_srv, 0);
	}
	else
	{
		CreateWICTextureFromFile(m_pDX->GetDevice(), ptr_model->GetTextureFileName().c_str(), nullptr, &texture_srv, 0);
	}

	if (texture_srv == nullptr)
	{
		JW_RELEASE(texture);

		m_vSharedTextureData.pop_back();
	}
}

auto JWSystemRender::GetSharedTexture(size_t Index) noexcept->ID3D11ShaderResourceView*
{
	ID3D11ShaderResourceView* result{};

	if (Index < m_vSharedTextureData.size())
	{
		result = m_vSharedTextureData[Index].TextureSRV;
	}

	return result;
}

auto JWSystemRender::CreateSharedModelFromModelData(const SModelData& ModelData) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.CreateMeshBuffers(ModelData, ERenderType::Model_Static);

	return &current_model;
}

auto JWSystemRender::CreateDynamicSharedModelFromModelData(const SModelData& ModelData) noexcept->JWModel*
{
	m_vSharedModel.push_back(JWModel());

	auto& current_model = m_vSharedModel[m_vSharedModel.size() - 1];

	current_model.Create(*m_pDX, m_BaseDirectory);

	JWPrimitiveMaker maker{};
	current_model.CreateMeshBuffers(ModelData, ERenderType::Model_Dynamic);

	return &current_model;
}

auto JWSystemRender::CreateSharedModelFromFile(ESharedModelType Type, STRING FileName) noexcept->JWModel*
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

auto JWSystemRender::GetSharedModel(size_t Index) noexcept->JWModel*
{
	JWModel* result{};

	if (Index < m_vSharedModel.size())
	{
		result = &m_vSharedModel[Index];
	}

	return result;
}

auto JWSystemRender::CreateSharedLineModel() noexcept->JWLineModel*
{
	m_vSharedLineModel.push_back(JWLineModel());

	auto& current_line_model = m_vSharedLineModel[m_vSharedLineModel.size() - 1];

	current_line_model.Create(*m_pDX);

	return &current_line_model;
}

auto JWSystemRender::GetSharedLineModel(size_t Index) noexcept->JWLineModel*
{
	JWLineModel* result{};

	if (Index < m_vSharedLineModel.size())
	{
		result = &m_vSharedLineModel[Index];
	}

	return result;
}

auto JWSystemRender::CreateSharedImage2D(SPositionInt Position, SSizeInt Size) noexcept->JWImage*
{
	m_vSharedImage2D.push_back(JWImage());

	auto& current_image = m_vSharedImage2D[m_vSharedImage2D.size() - 1];

	current_image.Create(*m_pDX);

	current_image.SetPosition(XMFLOAT2(static_cast<float>(Position.X), static_cast<float>(Position.Y)));
	current_image.SetSize(XMFLOAT2(static_cast<float>(Size.Width), static_cast<float>(Size.Height)));

	return &current_image;
}

auto JWSystemRender::GetSharedImage2D(size_t Index) noexcept->JWImage*
{
	JWImage* result{};

	if (Index < m_vSharedImage2D.size())
	{
		result = &m_vSharedImage2D[Index];
	}

	return result;
}

auto JWSystemRender::CreateSharedTerrainFromFile(const STRING& FileName, float HeightFactor) noexcept->STerrainData*
{
	auto new_terrain = m_TerrainGenerator.GenerateTerrainFromFile(FileName, HeightFactor);

	m_vSharedTerrain.push_back(new_terrain);

	return &m_vSharedTerrain[m_vSharedTerrain.size() - 1];
}

auto JWSystemRender::GetSharedTerrain(size_t Index) noexcept->STerrainData*
{
	STerrainData* result{};

	if (Index < m_vSharedTerrain.size())
	{
		result = &m_vSharedTerrain[Index];
	}

	return result;
}

void JWSystemRender::CreateAnimationTextureFromFile(STRING FileName) noexcept
{
	m_vAnimationTextureData.push_back(STextureData());

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

auto JWSystemRender::GetAnimationTexture(size_t Index) noexcept->STextureData*
{
	STextureData* result{};

	if (Index < m_vAnimationTextureData.size())
	{
		result = &m_vAnimationTextureData[Index];
	}

	return result;
}

void JWSystemRender::AddBoundingVolumeInstance(float Radius, const XMVECTOR& Translation) noexcept
{
	auto instance = m_pECS->SystemRender().BoundingVolume().ModelData.VertexData.PushInstance();

	auto mat_s = XMMatrixScaling(Radius, Radius, Radius);
	auto mat_t = XMMatrixTranslationFromVector(Translation);

	// Set the world matrix of the instance
	instance->World = XMMatrixIdentity() * mat_s * mat_t;

	UpdateBoundingVolumes();
}

void JWSystemRender::EraseBoundingVolumeInstance(uint32_t InstanceID) noexcept
{
	m_pECS->SystemRender().BoundingVolume().ModelData.VertexData.EraseInstanceAt(InstanceID);

	UpdateBoundingVolumes();
}

void JWSystemRender::UpdateBoundingVolumeInstance(uint32_t InstanceID, float Radius, const XMVECTOR& Translation) noexcept
{
	auto instance = m_pECS->SystemRender().BoundingVolume().ModelData.VertexData.GetInstance(InstanceID);

	auto mat_s = XMMatrixScaling(Radius, Radius, Radius);
	auto mat_t = XMMatrixTranslationFromVector(Translation);

	// Set the world matrix of the instance
	instance->World = XMMatrixIdentity() * mat_s * mat_t;

	UpdateBoundingVolumes();
}

PRIVATE inline void JWSystemRender::UpdateBoundingVolumes() noexcept
{
	m_pDX->UpdateDynamicResource(m_BoundingVolume.ModelVertexBuffer[KVBIDInstancing],
		m_BoundingVolume.ModelData.VertexData.GetInstancePtrData(),
		m_BoundingVolume.ModelData.VertexData.GetInstanceByteSize());
}

void JWSystemRender::Execute() noexcept
{
	m_FrustumCulledEntityCount = 0;

	// Opaque drawing
	m_pDX->SetBlendState(EBlendState::Opaque);

	for (auto& iter : m_vpComponents)
	{
		// Check transparency
		if (iter->FlagComponentRenderOption & JWFlagComponentRenderOption_UseTransparency)
		{
			continue;
		}

		// Check flag - View frustum drawing
		if (!(m_FlagSystemRenderOption & JWFlagSystemRenderOption_DrawViewFrustum))
		{
			if (iter->PtrEntity->GetEntityType() == EEntityType::ViewFrustum)
			{
				continue;
			}
		}

		// Check flag - Camera drawing
		auto camera = iter->PtrEntity->GetComponentCamera();
		if (camera)
		{
			// This is a Camera entity.

			if (!(m_FlagSystemRenderOption & JWFlagSystemRenderOption_DrawCameras))
			{
				continue;
			}

			if (camera->ComponentID == m_pECS->SystemCamera().GetCurrentCameraComponentID())
			{
				continue;
			}
		}

		// Check flag - Frustum culling
		if (m_FlagSystemRenderOption & JWFlagSystemRenderOption_UseFrustumCulling)
		{
			if (IsCulledByViewFrustum(iter))
			{
				continue;
			}
		}

		// Check flag - Rasterizer state
		if (iter->FlagComponentRenderOption & JWFlagComponentRenderOption_AlwaysSolidNoCull)
		{
			m_pDX->SetRasterizerState(ERasterizerState::SolidNoCull);
		}
		else
		{
			m_pDX->SetRasterizerState(m_UniversalRasterizerState);
		}

		// Set depth stencil state for the component
		m_pDX->SetDepthStencilState(iter->DepthStencilState);

		if (iter->FlagComponentRenderOption & JWFlagComponentRenderOption_UseGPUAnimation)
		{
			// GPU animation
			// Real animationing occurs in vertex shader when Draw() is called.
			AnimateOnGPU(*iter);
		}
		else
		{
			// CPU animation
			AnimateOnCPU(*iter);
		}

		SetShaders(*iter);

		Draw(*iter);
	}


	// Bounding volume drawing
	if (m_FlagSystemRenderOption & JWFlagSystemRenderOption_DrawBoundingVolumes)
	{
		DrawInstancedBoundingVolume();
	}


	// Transparent drawing
	m_pDX->SetBlendState(EBlendState::Transprent);

	for (auto& iter : m_vpComponents)
	{
		// Check transparency
		if (!(iter->FlagComponentRenderOption & JWFlagComponentRenderOption_UseTransparency))
		{
			continue;
		}

		// Check flag - View frustum drawing
		if (!(m_FlagSystemRenderOption & JWFlagSystemRenderOption_DrawViewFrustum))
		{
			if (iter->PtrEntity->GetEntityType() == EEntityType::ViewFrustum)
			{
				continue;
			}
		}

		// Check flag - Camera drawing
		auto camera = iter->PtrEntity->GetComponentCamera();
		if (camera)
		{
			// This is a Camera entity.

			if (!(m_FlagSystemRenderOption & JWFlagSystemRenderOption_DrawCameras))
			{
				continue;
			}

			if (camera->ComponentID == m_pECS->SystemCamera().GetCurrentCameraComponentID())
			{
				continue;
			}
		}

		// Check flag - Frustum culling
		if (m_FlagSystemRenderOption & JWFlagSystemRenderOption_UseFrustumCulling)
		{
			if (IsCulledByViewFrustum(iter))
			{
				continue;
			}
		}

		// Check flag - Rasterizer state
		if (iter->FlagComponentRenderOption & JWFlagComponentRenderOption_AlwaysSolidNoCull)
		{
			m_pDX->SetRasterizerState(ERasterizerState::SolidNoCull);
		}
		else
		{
			m_pDX->SetRasterizerState(m_UniversalRasterizerState);
		}

		// Set depth stencil state for the component
		m_pDX->SetDepthStencilState(iter->DepthStencilState);

		if (iter->FlagComponentRenderOption & JWFlagComponentRenderOption_UseGPUAnimation)
		{
			// GPU animation
			// Real animationing occurs in vertex shader when Draw() is called.
			AnimateOnGPU(*iter);
		}
		else
		{
			// CPU animation
			AnimateOnCPU(*iter);
		}

		SetShaders(*iter);

		Draw(*iter);
	}
}

PRIVATE auto JWSystemRender::IsCulledByViewFrustum(const SComponentRender* pRender) const noexcept->bool
{
	auto physics = pRender->PtrEntity->GetComponentPhysics();
	if (physics == nullptr)
	{
		return false;
	}

	m_pECS->SystemCamera().CaptureViewFrustum();
	auto frustum = m_pECS->SystemCamera().GetCapturedViewFrustum();

	auto& center_pos = physics->BoundingSphereData.Center;
	auto radius = physics->BoundingSphereData.Radius;

	// Left plane
	auto lp_v0 = XMVector3Normalize(frustum.FLU - frustum.NLU);
	auto lp_v1 = XMVector3Normalize(frustum.FLD - frustum.NLU);
	auto lp_vn = XMVector3Normalize(XMVector3Cross(lp_v1, lp_v0)); // @important

	auto cmp_pos = center_pos + (-lp_vn) * radius;
	auto cmp_vec = XMVector3Normalize(cmp_pos - frustum.NLU);
	auto distance = XMVector3Dot(cmp_vec, lp_vn);

	if (XMVector3Greater(distance, XMVectorZero()))
	{
		// Entity is out of the view frustum.
		++m_FrustumCulledEntityCount;

		return true;
	}

	// Right plane
	auto rp_v0 = XMVector3Normalize(frustum.FRU - frustum.NRU);
	auto rp_v1 = XMVector3Normalize(frustum.FRD - frustum.NRU);
	auto rp_vn = XMVector3Normalize(XMVector3Cross(rp_v0, rp_v1)); // @important

	cmp_pos = center_pos + (-rp_vn) * radius;
	cmp_vec = XMVector3Normalize(cmp_pos - frustum.NRU);
	distance = XMVector3Dot(cmp_vec, rp_vn);

	if (XMVector3Greater(distance, XMVectorZero()))
	{
		// Entity is out of the view frustum.
		++m_FrustumCulledEntityCount;

		return true;
	}

	return false;
}

void JWSystemRender::DrawInstancedBoundingVolume() noexcept
{
	auto world_matrix{ XMMatrixIdentity() };

	// Set RS State
	m_pDX->SetRasterizerState(ERasterizerState::WireFrame);

	// Set VS Base
	m_pDX->SetVS(EVertexShader::VSBase);
	
	// Update VS constant buffer #0
	m_VSCBSpace.WVP = XMMatrixTranspose(world_matrix * m_pECS->SystemCamera().CurrentViewProjectionMatrix());
	m_VSCBSpace.World = XMMatrixTranspose(world_matrix);
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Update VS constant buffer #1
	m_VSCBFlags.FlagVS = JWFlagVS_Instanced;
	m_pDX->UpdateVSCBFlags(m_VSCBFlags);

	// Set PS Base
	m_pDX->SetPS(EPixelShader::PSBase);

	// Update PS constant buffer
	m_pDX->UpdatePSCBFlags(0);

	// Set IA primitive topology
	m_pDX->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 3, m_BoundingVolume.ModelVertexBuffer,
		m_BoundingVolume.ModelData.VertexData.GetPtrStrides(), m_BoundingVolume.ModelData.VertexData.GetPtrOffsets());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_BoundingVolume.ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw indexed instanced
	m_pDX->GetDeviceContext()->DrawIndexedInstanced(
		m_BoundingVolume.ModelData.IndexData.GetCount(), m_BoundingVolume.ModelData.VertexData.GetInstanceCount(), 0, 0, 0);
}

void JWSystemRender::DrawNonInstancedBoundingVolumes(SComponentRender& Component) noexcept
{
	auto entity = Component.PtrEntity;
	auto transform = entity->GetComponentTransform();
	auto physics = entity->GetComponentPhysics();

	if (physics)
	{
		assert(transform);

		const auto& position = transform->Position;
		const auto& radius = physics->BoundingSphereData.Radius;
		auto world_matrix{ XMMatrixScaling(radius, radius, radius) };
		world_matrix *= XMMatrixTranslationFromVector(position);

		// Set VS Base
		m_pDX->SetVS(EVertexShader::VSBase);

		// Update VS constant buffer
		m_VSCBSpace.WVP = XMMatrixTranspose(world_matrix * m_pECS->SystemCamera().CurrentViewProjectionMatrix());
		m_VSCBSpace.World = XMMatrixTranspose(world_matrix);
		m_pDX->UpdateVSCBSpace(m_VSCBSpace);

		// Set PS Base
		m_pDX->SetPS(EPixelShader::PSBase);

		// Update PS constant buffer
		m_pDX->UpdatePSCBFlags(0);

		// Set IA primitive topology
		m_pDX->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);

		// Set IA vertex buffer
		m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, m_BoundingVolume.ModelVertexBuffer,
			m_BoundingVolume.ModelData.VertexData.GetPtrStrides(), m_BoundingVolume.ModelData.VertexData.GetPtrOffsets());

		// Set IA index buffer
		m_pDX->GetDeviceContext()->IASetIndexBuffer(m_BoundingVolume.ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw
		m_pDX->GetDeviceContext()->DrawIndexed(m_BoundingVolume.ModelData.IndexData.GetCount(), 0, 0);
	}
}

void JWSystemRender::AnimateOnGPU(SComponentRender& Component) noexcept
{
	auto type = Component.RenderType;
	if (type != ERenderType::Model_Rigged) { return; }

	auto& anim_state = Component.AnimationState;
	auto& model = Component.PtrModel;

	// Set VS texture
	m_pDX->GetDeviceContext()->VSSetShaderResources(0, 1, &Component.PtrAnimationTexture->TextureSRV);

	if (anim_state.CurrAnimationID > 0)
	{
		// Not TPose

		auto& current_anim{ model->ModelData.AnimationSet.vAnimations[anim_state.CurrAnimationID - 1] };

		// Advance animation tick
		//anim_state.CurrAnimationTick += model->RiggedModelData.AnimationSet.vAnimations[current_animation_id - 1].AnimationTicksPerGameTick;
		anim_state.CurrAnimationTick += 2.0f;

		// Reset tick if the animation is over.
		if (anim_state.CurrAnimationTick >= current_anim.TotalAnimationTicks)
		{
			anim_state.CurrAnimationTick = 0;

			Component.SetAnimation(anim_state.NextAnimationID);
		}

		// Calculate current animation time for interpolation
		anim_state.CurrFrameTime = anim_state.CurrAnimationTick - fmodf(anim_state.CurrAnimationTick, current_anim.AnimationTicksPerGameTick);

		// Calculate next animation time for interpolation
		anim_state.NextFrameTime = anim_state.CurrFrameTime + current_anim.AnimationTicksPerGameTick;

		// Interpolation factor Delta's range is [0.0, 1.0]
		anim_state.TweeningTime = (anim_state.CurrAnimationTick - anim_state.CurrFrameTime) / current_anim.AnimationTicksPerGameTick;

		// Constrain next animation time
		if (anim_state.NextFrameTime >= current_anim.TotalAnimationTicks)
		{
			anim_state.NextFrameTime = 0.0f;
		}

		m_VSCBGPUAnimation.CurrFrame = static_cast<uint32_t>(anim_state.CurrFrameTime / current_anim.AnimationTicksPerGameTick);
		m_VSCBGPUAnimation.NextFrame = static_cast<uint32_t>(anim_state.NextFrameTime / current_anim.AnimationTicksPerGameTick);
	}
	else
	{
		// TPose
		anim_state.CurrFrameTime = 0.0f;
		anim_state.NextFrameTime = 0.0f;
		anim_state.TweeningTime = 0;

		m_VSCBGPUAnimation.CurrFrame = 0;
		m_VSCBGPUAnimation.NextFrame = 0;
	}

	// Update constant buffer for GPU
	m_VSCBGPUAnimation.AnimationID = anim_state.CurrAnimationID;
	m_VSCBGPUAnimation.DeltaTime = anim_state.TweeningTime;
}

void JWSystemRender::AnimateOnCPU(SComponentRender& Component) noexcept
{
	auto type = Component.RenderType;
	if (type != ERenderType::Model_Rigged) { return; }
	
	auto& anim_state = Component.AnimationState;
	auto& model = Component.PtrModel;

	if ((Component.FlagComponentRenderOption & JWFlagComponentRenderOption_DrawTPose) || (anim_state.CurrAnimationID == 0))
	{
		// TPose

		anim_state.CurrFrameTime = 0.0f;
		anim_state.NextFrameTime = 0.0f;
		anim_state.TweeningTime = 0.0f;

		UpdateNodeTPoseIntoBones(anim_state.CurrAnimationTick, model->ModelData, model->ModelData.NodeTree.vNodes[0],
			XMMatrixIdentity());

		// Update bone's T-Pose transformation for shader's constant buffer
		for (size_t iterator_bone_mat{}; iterator_bone_mat < model->ModelData.BoneTree.vBones.size(); ++iterator_bone_mat)
		{
			m_VSCBCPUAnimation.TransformedBoneMatrices[iterator_bone_mat] =
				XMMatrixTranspose(model->ModelData.BoneTree.vBones[iterator_bone_mat].FinalTransformation);
		}
	}
	else
	{
		// Not TPose

		auto& current_anim{ model->ModelData.AnimationSet.vAnimations[anim_state.CurrAnimationID - 1] };

		// Advance animation tick
		//anim_state.CurrAnimationTick += model->RiggedModelData.AnimationSet.vAnimations[anim_state.CurrAnimationID - 1].AnimationTicksPerGameTick;
		anim_state.CurrAnimationTick += 2.0f;

		// Reset tick if the animation is over.
		if (anim_state.CurrAnimationTick >= current_anim.TotalAnimationTicks)
		{
			anim_state.CurrAnimationTick = 0;

			Component.SetAnimation(anim_state.NextAnimationID);
		}

		// Update bones' transformations for the animation.
		UpdateNodeAnimationIntoBones((Component.FlagComponentRenderOption & JWFlagComponentRenderOption_UseAnimationInterpolation),
			anim_state, model->ModelData, model->ModelData.NodeTree.vNodes[0], XMMatrixIdentity());

		// Update bone's final transformation for shader's constant buffer
		for (size_t iterator_bone_mat{}; iterator_bone_mat < model->ModelData.BoneTree.vBones.size(); ++iterator_bone_mat)
		{
			m_VSCBCPUAnimation.TransformedBoneMatrices[iterator_bone_mat] =
				XMMatrixTranspose(model->ModelData.BoneTree.vBones[iterator_bone_mat].FinalTransformation);
		}
	}
}

PRIVATE void JWSystemRender::UpdateNodeAnimationIntoBones(bool UseInterpolation, SAnimationState& AnimationState, SModelData& ModelData,
	const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept
{
	XMMATRIX global_transformation = CurrentNode.Transformation * Accumulated;

	if (CurrentNode.BoneID >= 0)
	{
		auto& bone = ModelData.BoneTree.vBones[CurrentNode.BoneID];
		auto& current_animation = ModelData.AnimationSet.vAnimations[AnimationState.CurrAnimationID - 1];

		// Calculate current frame time for interpolation
		AnimationState.CurrFrameTime = AnimationState.CurrAnimationTick 
			- fmodf(AnimationState.CurrAnimationTick, current_animation.AnimationTicksPerGameTick);

		// Calculate next frame time for interpolation
		AnimationState.NextFrameTime = AnimationState.CurrFrameTime + current_animation.AnimationTicksPerGameTick;

		// Interpolation factor DeltaTime's range is [0.0, 1.0]
		AnimationState.TweeningTime = (AnimationState.CurrAnimationTick - AnimationState.CurrFrameTime) / current_animation.AnimationTicksPerGameTick;

		// Constrain next frame time
		if (AnimationState.NextFrameTime >= current_animation.TotalAnimationTicks)
		{
			AnimationState.NextFrameTime = 0;
		}

		for (const auto& CurrentNode_animation : current_animation.vNodeAnimation)
		{
			if (CurrentNode_animation.NodeID == CurrentNode.ID)
			{
				XMMATRIX matrix_scaling{};
				XMVECTOR scaling_key_a{};
				XMVECTOR scaling_key_b{};
				XMVECTOR scaling_interpolated{};

				XMMATRIX matrix_rotation{};
				XMVECTOR rotation_key_a{};
				XMVECTOR rotation_key_b{};
				XMVECTOR rotation_interpolated{};

				XMMATRIX matrix_translation{};
				XMVECTOR translation_key_a{};
				XMVECTOR translation_key_b{};
				XMVECTOR translation_interpolated{};

				// #1. Find scaling keys
				for (const auto& key : CurrentNode_animation.vKeyScaling)
				{
					if (key.TimeInTicks <= AnimationState.CurrFrameTime)
					{
						scaling_key_a = XMLoadFloat3(&key.Key);
					}
					if (key.TimeInTicks <= AnimationState.NextFrameTime)
					{
						scaling_key_b = XMLoadFloat3(&key.Key);
					}
				}
				// Linear interpolation
				scaling_interpolated = scaling_key_a + (AnimationState.TweeningTime * (scaling_key_b - scaling_key_a));

				// #2. Find rotation keys
				for (const auto& key : CurrentNode_animation.vKeyRotation)
				{
					if (key.TimeInTicks <= AnimationState.CurrFrameTime)
					{
						rotation_key_a = key.Key;
					}
					if (key.TimeInTicks <= AnimationState.NextFrameTime)
					{
						rotation_key_b = key.Key;
					}
				}
				// Spherical linear interpolation!
				rotation_interpolated = XMQuaternionSlerp(rotation_key_a, rotation_key_b, AnimationState.TweeningTime);

				// #3. Find translation keys
				for (const auto& key : CurrentNode_animation.vKeyPosition)
				{
					if (key.TimeInTicks <= AnimationState.CurrFrameTime)
					{
						translation_key_a = XMLoadFloat3(&key.Key);
					}
					if (key.TimeInTicks <= AnimationState.NextFrameTime)
					{
						translation_key_b = XMLoadFloat3(&key.Key);
					}
				}
				// Linear interpolation
				translation_interpolated = translation_key_a + (AnimationState.TweeningTime * (translation_key_b - translation_key_a));

				if (!UseInterpolation)
				{
					scaling_interpolated = scaling_key_a;
					rotation_interpolated = rotation_key_a;
					translation_interpolated = translation_key_a;
				}

				matrix_scaling = XMMatrixScalingFromVector(scaling_interpolated);
				matrix_rotation = XMMatrixRotationQuaternion(rotation_interpolated);
				matrix_translation = XMMatrixTranslationFromVector(translation_interpolated);

				global_transformation = matrix_scaling * matrix_rotation * matrix_translation * Accumulated;

				break;
			}
		}

		bone.FinalTransformation = bone.Offset * global_transformation;
	}

	if (CurrentNode.vChildrenID.size())
	{
		for (auto child_id : CurrentNode.vChildrenID)
		{
			UpdateNodeAnimationIntoBones(UseInterpolation, AnimationState, ModelData,
				ModelData.NodeTree.vNodes[child_id], global_transformation);
		}
	}
}

PRIVATE void JWSystemRender::UpdateNodeTPoseIntoBones(float AnimationTime, SModelData& ModelData, const SModelNode& CurrentNode,
	const XMMATRIX Accumulated) noexcept
{
	XMMATRIX accumulation = CurrentNode.Transformation * Accumulated;

	if (CurrentNode.BoneID >= 0)
	{
		auto& bone = ModelData.BoneTree.vBones[CurrentNode.BoneID];

		bone.FinalTransformation = bone.Offset * accumulation;
	}

	if (CurrentNode.vChildrenID.size())
	{
		for (auto child_id : CurrentNode.vChildrenID)
		{
			UpdateNodeTPoseIntoBones(AnimationTime, ModelData, ModelData.NodeTree.vNodes[child_id], accumulation);
		}
	}
}

void JWSystemRender::SetShaders(SComponentRender& Component) noexcept
{
	// Set PS
	m_pDX->SetPS(Component.PixelShader);

	// Update PS constant buffer (if necessary)
	if (Component.PixelShader == EPixelShader::PSBase)
	{
		// Clean the flag
		m_PSCBFlags.FlagPS = 0;

		bool use_lighting = Component.FlagComponentRenderOption & JWFlagComponentRenderOption_GetLit;
		if (!(m_FlagSystemRenderOption & JWFlagSystemRenderOption_UseLighting))
		{
			use_lighting = false;
		}

		if (use_lighting)
		{
			m_PSCBFlags.FlagPS |= JWFlagPS_UseLighting;
		}

		if (Component.FlagComponentRenderOption & JWFlagComponentRenderOption_UseDiffuseTexture)
		{
			m_PSCBFlags.FlagPS |= JWFlagPS_UseDiffuseTexture;
		}

		if (Component.FlagComponentRenderOption & JWFlagComponentRenderOption_UseNormalTexture)
		{
			m_PSCBFlags.FlagPS |= JWFlagPS_UseNormalTexture;
		}

		m_pDX->UpdatePSCBFlags(m_PSCBFlags);
	}

	// Set PS textures & sampler
	if (Component.FlagComponentRenderOption & JWFlagComponentRenderOption_UseDiffuseTexture)
	{
		// Set PS texture (diffuse)
		m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &Component.PtrTextureDiffuse);
	}

	if (Component.FlagComponentRenderOption & JWFlagComponentRenderOption_UseNormalTexture)
	{
		// Set PS texture (normal)
		m_pDX->GetDeviceContext()->PSSetShaderResources(1, 1, &Component.PtrTextureNormal);
	}

	// Set PS texture sampler
	m_pDX->SetPSSamplerState(ESamplerState::MinMagMipLinearWrap);

	// Set VS
	m_pDX->SetVS(Component.VertexShader);

	// @important
	// Get transform component, if there is.
	const auto& component_transform = Component.PtrEntity->GetComponentTransform();
	XMMATRIX component_world_matrix{ XMMatrixIdentity() };
	if (component_transform)
	{
		component_world_matrix = component_transform->WorldMatrix;
	}

	// Update VS constant buffer
	XMMATRIX WVP{}, World{};
	WVP = XMMatrixTranspose(component_world_matrix * m_pECS->SystemCamera().CurrentViewProjectionMatrix());
	World = XMMatrixTranspose(component_world_matrix);

	switch (Component.RenderType)
	{
	case ERenderType::Model_Static:
		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Model_Dynamic:
		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Model_Rigged:
		if (Component.FlagComponentRenderOption & JWFlagComponentRenderOption_UseGPUAnimation)
		{
			m_VSCBFlags.FlagVS = JWFlagVS_UseAnimation | JWFlagVS_AnimateOnGPU;
			
			m_pDX->UpdateVSCBGPUAnimationData(m_VSCBGPUAnimation);
		}
		else
		{
			m_VSCBFlags.FlagVS = JWFlagVS_UseAnimation;
			
			m_pDX->UpdateVSCBCPUAnimationData(m_VSCBCPUAnimation);
		}
		break;
	case ERenderType::Image_2D:
		WVP = XMMatrixTranspose(m_pECS->SystemCamera().OrthographicMatrix());

		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Model_Line3D:
		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Model_Line2D:
		WVP = XMMatrixTranspose(component_world_matrix * m_pECS->SystemCamera().OrthographicMatrix());

		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Terrain:
		m_VSCBFlags.FlagVS = 0;
		break;
	default:
		break;
	}

	m_VSCBSpace.WVP = WVP;
	m_VSCBSpace.World = World;

	m_pDX->UpdateVSCBSpace(m_VSCBSpace);
	m_pDX->UpdateVSCBFlags(m_VSCBFlags);
}

PRIVATE void JWSystemRender::Draw(SComponentRender& Component) noexcept
{
	auto ptr_device_context = m_pDX->GetDeviceContext();

	auto type = Component.RenderType;
	auto& model = Component.PtrModel;
	auto& image = Component.PtrImage;
	auto& line = Component.PtrLine;

	// Set OM blend state
	if (Component.FlagComponentRenderOption & JWFlagComponentRenderOption_UseTransparency)
	{
		m_pDX->SetBlendState(EBlendState::Transprent);
	}
	else
	{
		m_pDX->SetBlendState(EBlendState::Opaque);
	}

	// Set IA primitive topology 
	if ((type == ERenderType::Model_Static) || (type == ERenderType::Model_Dynamic) ||
		(type == ERenderType::Model_Rigged) || (type == ERenderType::Image_2D) ||
		(type == ERenderType::Terrain))
	{
		m_pDX->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);
	}
	else if ((type == ERenderType::Model_Line3D) || (type == ERenderType::Model_Line2D))
	{
		m_pDX->SetPrimitiveTopology(EPrimitiveTopology::LineList);
	}
	
	switch (type)
	{
	case ERenderType::Model_Static:
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, model->ModelVertexBuffer, model->ModelData.VertexData.GetPtrStrides(), model->ModelData.VertexData.GetPtrOffsets());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(model->ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(model->ModelData.IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Model_Dynamic:
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, model->ModelVertexBuffer, model->ModelData.VertexData.GetPtrStrides(), model->ModelData.VertexData.GetPtrOffsets());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(model->ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(model->ModelData.IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Model_Rigged:
		//
		// @important!! (Buffer count = 2)
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 2, model->ModelVertexBuffer, model->ModelData.VertexData.GetPtrStrides(), model->ModelData.VertexData.GetPtrOffsets());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(model->ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(model->ModelData.IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Image_2D:
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, &image->m_VertexBuffer, image->m_VertexData.GetPtrStrides(), image->m_VertexData.GetPtrOffsets());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(image->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(image->m_IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Model_Line3D:
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, &line->m_VertexBuffer, line->m_VertexData.GetPtrStrides(), line->m_VertexData.GetPtrOffsets());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(line->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(line->m_IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Model_Line2D:
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, &line->m_VertexBuffer, line->m_VertexData.GetPtrStrides(), line->m_VertexData.GetPtrOffsets());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(line->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(line->m_IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Terrain:
		for (auto& iter : Component.PtrTerrain->QuadTree)
		{
			if (iter.IsMeshNode)
			{
				// Set IA vertex buffer
				ptr_device_context->IASetVertexBuffers(
					0, 1, &iter.VertexBuffer, iter.VertexData.GetPtrStrides(), iter.VertexData.GetPtrOffsets());

				// Set IA index buffer
				ptr_device_context->IASetIndexBuffer(iter.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

				// Draw indexed
				ptr_device_context->DrawIndexed(iter.IndexData.GetCount(), 0, 0);
			}
		}
		break;
	default:
		break;
	}

	// Draw normals
	if (m_FlagSystemRenderOption & JWFlagSystemRenderOption_DrawNormals)
	{
		DrawNormals(Component);
	}
}

PRIVATE void JWSystemRender::DrawNormals(SComponentRender& Component) noexcept
{
	auto& model = Component.PtrModel;
	auto& terrain = Component.PtrTerrain;

	if ((model == nullptr) && (terrain == nullptr)) { return; }

	const auto& component_transform = Component.PtrEntity->GetComponentTransform();
	if (component_transform == nullptr) { return; }
	
	const auto& world_matrix{ component_transform->WorldMatrix };
	
	// Set VS Base
	m_pDX->SetVS(EVertexShader::VSBase);

	// Update VS constant buffer
	m_VSCBSpace.WVP = XMMatrixTranspose(world_matrix * m_pECS->SystemCamera().CurrentViewProjectionMatrix());
	m_VSCBSpace.World = XMMatrixTranspose(world_matrix);
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Set PS Base
	m_pDX->SetPS(EPixelShader::PSBase);

	// Update PS constant buffer
	m_pDX->UpdatePSCBFlags(0);

	// Set IA primitive topology
	m_pDX->SetPrimitiveTopology(EPrimitiveTopology::LineList);

	if (model)
	{
		// Set IA vertex buffer
		m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &model->NormalVertexBuffer,
			model->NormalData.VertexData.GetPtrStrides(), model->NormalData.VertexData.GetPtrOffsets());

		// Set IA index buffer
		m_pDX->GetDeviceContext()->IASetIndexBuffer(model->NormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw
		m_pDX->GetDeviceContext()->DrawIndexed(model->NormalData.IndexData.GetCount(), 0, 0);
	}

	if (terrain)
	{
		for (auto& iter : terrain->QuadTree)
		{
			if (iter.IsMeshNode)
			{
				// Set IA vertex buffer
				m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &iter.NormalVertexBuffer,
					iter.NormalData.VertexData.GetPtrStrides(), iter.NormalData.VertexData.GetPtrOffsets());

				// Set IA index buffer
				m_pDX->GetDeviceContext()->IASetIndexBuffer(iter.NormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

				// Draw
				m_pDX->GetDeviceContext()->DrawIndexed(iter.NormalData.IndexData.GetCount(), 0, 0);
			}
		}
	}
	
}

void JWSystemRender::SetUniversalRasterizerState(ERasterizerState State) noexcept
{
	m_UniversalRasterizerState = State;
}

void JWSystemRender::SetSystemRenderFlag(JWFlagSystemRenderOption Flag) noexcept
{
	m_FlagSystemRenderOption = Flag;
}

void JWSystemRender::ToggleSystemRenderFlag(JWFlagSystemRenderOption Flag) noexcept
{
	m_FlagSystemRenderOption ^= Flag;
}

void JWSystemRender::ToggleWireFrame() noexcept
{
	if (m_UniversalRasterizerState == ERasterizerState::WireFrame)
	{
		m_UniversalRasterizerState = m_OldUniversalRasterizerState;
		m_OldUniversalRasterizerState = ERasterizerState::WireFrame;
	}
	else
	{
		m_OldUniversalRasterizerState = m_UniversalRasterizerState;
		m_UniversalRasterizerState = ERasterizerState::WireFrame;
	}
}
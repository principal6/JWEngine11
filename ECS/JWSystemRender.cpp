#include "JWECS.h"
#include "../Core/JWDX.h"
#include "../Core/JWCamera.h"
#include "../Core/JWPrimitiveMaker.h"

using namespace JWEngine;

void JWSystemRender::Create(JWECS& ECS, JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept
{
	// Set JWECS pointer.
	m_pECS = &ECS;

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;
	
	// Set base directory.
	m_BaseDirectory = BaseDirectory;

	// Bounding volume (with instance buffer)
	JWPrimitiveMaker primitive{};
	m_BoundingVolume.Create(DX, BaseDirectory);
	m_BoundingVolume.CreateMeshBuffers(primitive.MakeSphere(1.0f, 16, 7), ERenderType::Model_Static);
	m_BoundingVolume.CreateInstanceBuffer();
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

	m_BoundingVolume.Destroy();
}

auto JWSystemRender::CreateComponent() noexcept->SComponentRender&
{
	uint32_t slot{ static_cast<uint32_t>(m_vpComponents.size()) };

	auto new_entry{ new SComponentRender() };
	m_vpComponents.push_back(new_entry);

	// @important
	// Save component ID & set default data
	m_vpComponents[slot]->ComponentID = slot;

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
		m_vpComponents[last_index] = nullptr;
	}

	m_vpComponents.pop_back();
}

void JWSystemRender::AddBoundingVolumeInstance(float Radius, const XMFLOAT3& Translation) noexcept
{
	auto instance = m_pECS->SystemRender().BoundingVolume().ModelData.VertexData.PushInstance();

	auto mat_s = XMMatrixScaling(Radius, Radius, Radius);
	auto mat_t = XMMatrixTranslation(Translation.x, Translation.y, Translation.z);

	// Set the world matrix of the instance
	instance->World = XMMatrixIdentity() * mat_s * mat_t;

	UpdateBoundingVolume();
}

void JWSystemRender::UpdateBoundingVolume() noexcept
{
	m_pDX->UpdateDynamicResource(m_BoundingVolume.ModelVertexBuffer[KVBIDInstancing],
		m_BoundingVolume.ModelData.VertexData.GetInstancePtrData(),
		m_BoundingVolume.ModelData.VertexData.GetInstanceByteSize());
}

void JWSystemRender::Execute() noexcept
{
	for (auto& iter : m_vpComponents)
	{
		if (iter->FlagRenderOption & JWFlagRenderOption_AlwaysSolidNoCull)
		{
			m_pDX->SetRasterizerState(ERasterizerState::SolidNoCull);
		}
		else
		{
			m_pDX->SetRasterizerState(m_UniversalRasterizerState);
		}

		// Set blend state for the component
		m_pDX->SetBlendState(iter->BlendState);

		// Set depth stencil state for the component
		m_pDX->SetDepthStencilState(iter->DepthStencilState);

		if (iter->FlagRenderOption & JWFlagRenderOption_UseGPUAnimation)
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

	DrawInstancedBoundingVolume();
}

void JWSystemRender::DrawInstancedBoundingVolume() noexcept
{
	auto world_matrix{ XMMatrixIdentity() };

	// Set RS State
	m_pDX->SetRasterizerState(ERasterizerState::WireFrame);

	// Set VS Base
	m_pDX->SetVS(EVertexShader::VSBase);

	// Update VS constant buffer #0
	m_VSCBSpace.WVP = XMMatrixTranspose(world_matrix * m_pCamera->GetViewProjectionMatrix());
	m_VSCBSpace.World = XMMatrixTranspose(world_matrix);
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Update VS constant buffer #1
	m_VSCBFlags.FlagVS = JWFlagVS_Instanced;
	m_pDX->UpdateVSCBFlags(m_VSCBFlags);

	// Set PS Base
	m_pDX->SetPS(EPixelShader::PSBase);

	// Update PS constant buffer
	m_pDX->UpdatePSCBFlags(false, false);

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

void JWSystemRender::DrawBoundingVolumesNoInstancing(SComponentRender& Component) noexcept
{
	auto entity = Component.PtrEntity;
	auto transform = entity->GetComponentTransform();
	auto physics = entity->GetComponentPhysics();

	if (physics)
	{
		assert(transform);

		const auto& p = transform->Position;
		const auto& r = physics->BoundingSphereRadius;
		auto world_matrix{ XMMatrixScaling(r, r, r) };
		world_matrix *= XMMatrixTranslation(p.x, p.y, p.z);

		// Set VS Base
		m_pDX->SetVS(EVertexShader::VSBase);

		// Update VS constant buffer
		m_VSCBSpace.WVP = XMMatrixTranspose(world_matrix * m_pCamera->GetViewProjectionMatrix());
		m_VSCBSpace.World = XMMatrixTranspose(world_matrix);
		m_pDX->UpdateVSCBSpace(m_VSCBSpace);

		// Set PS Base
		m_pDX->SetPS(EPixelShader::PSBase);

		// Update PS constant buffer
		m_pDX->UpdatePSCBFlags(false, false);

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

	if ((Component.FlagRenderOption & JWFlagRenderOption_DrawTPose) || (anim_state.CurrAnimationID == 0))
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
		UpdateNodeAnimationIntoBones((Component.FlagRenderOption & JWFlagRenderOption_UseAnimationInterpolation),
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
		bool use_lighting = Component.FlagRenderOption & JWFlagRenderOption_UseLighting;
		if (!m_ShouldLight)
		{
			use_lighting = false;
		}

		m_pDX->UpdatePSCBFlags(
			(Component.FlagRenderOption & JWFlagRenderOption_UseTexture),
			(use_lighting)
		);
	}

	// If it uses texture
	if (Component.FlagRenderOption & JWFlagRenderOption_UseTexture)
	{
		// Set PS texture
		m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &Component.PtrTexture);

		// Set PS texture sampler
		m_pDX->SetPSSamplerState(ESamplerState::MinMagMipLinearWrap);
	}

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
	switch (Component.RenderType)
	{
	case ERenderType::Model_Static:
		WVP = XMMatrixTranspose(component_world_matrix * m_pCamera->GetViewProjectionMatrix());
		World = XMMatrixTranspose(component_world_matrix);

		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Model_Dynamic:
		WVP = XMMatrixTranspose(component_world_matrix * m_pCamera->GetViewProjectionMatrix());
		World = XMMatrixTranspose(component_world_matrix);

		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Model_Rigged:
		WVP = XMMatrixTranspose(component_world_matrix * m_pCamera->GetViewProjectionMatrix());
		World = XMMatrixTranspose(component_world_matrix);

		if (Component.FlagRenderOption & JWFlagRenderOption_UseGPUAnimation)
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
		WVP = XMMatrixTranspose(m_pCamera->GetTransformedOrthographicMatrix());

		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Model_Line3D:
		WVP = XMMatrixTranspose(component_world_matrix * m_pCamera->GetViewProjectionMatrix());

		m_VSCBFlags.FlagVS = 0;
		break;
	case ERenderType::Model_Line2D:
		WVP = XMMatrixTranspose(component_world_matrix * m_pCamera->GetFixedOrthographicMatrix());

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

	if (Component.FlagRenderOption & JWFlagRenderOption_UseTransparency)
	{
		m_pDX->SetBlendState(EBlendState::Transprent);
	}
	else
	{
		m_pDX->SetBlendState(EBlendState::Opaque);
	}

	if ((type == ERenderType::Model_Static) || (type == ERenderType::Model_Dynamic) ||
		(type == ERenderType::Model_Rigged) || (type == ERenderType::Image_2D))
	{
		// Set IA primitive topology
		m_pDX->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);
	}
	else if ((type == ERenderType::Model_Line3D) || (type == ERenderType::Model_Line2D))
	{
		// Set IA primitive topology
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
	default:
		break;
	}

	// Draw normals
	if (m_ShouldDrawNormals)
	{
		DrawNormals(Component);
	}
}

PRIVATE void JWSystemRender::DrawNormals(SComponentRender& Component) noexcept
{
	auto& model = Component.PtrModel;
	if (model == nullptr) { return; }

	const auto& component_transform = Component.PtrEntity->GetComponentTransform();
	if (component_transform == nullptr) { return; }
	
	const auto& world_matrix{ component_transform->WorldMatrix };
	
	// Set VS Base
	m_pDX->SetVS(EVertexShader::VSBase);

	// Update VS constant buffer
	m_VSCBSpace.WVP = XMMatrixTranspose(world_matrix * m_pCamera->GetViewProjectionMatrix());
	m_VSCBSpace.World = XMMatrixTranspose(world_matrix);
	m_pDX->UpdateVSCBSpace(m_VSCBSpace);

	// Set PS Base
	m_pDX->SetPS(EPixelShader::PSBase);

	// Update PS constant buffer
	m_pDX->UpdatePSCBFlags(false, false);

	// Set IA primitive topology
	m_pDX->SetPrimitiveTopology(EPrimitiveTopology::LineList);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &model->NormalVertexBuffer,
		model->NormalData.VertexData.GetPtrStrides(), model->NormalData.VertexData.GetPtrOffsets());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(model->NormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(model->NormalData.IndexData.GetCount(), 0, 0);
}

void JWSystemRender::SetUniversalRasterizerState(ERasterizerState State) noexcept
{
	m_UniversalRasterizerState = State;
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

void JWSystemRender::ToggleNormalDrawing() noexcept
{
	m_ShouldDrawNormals = !m_ShouldDrawNormals;
}

void JWSystemRender::ToggleLighting() noexcept
{
	m_ShouldLight = !m_ShouldLight;
}
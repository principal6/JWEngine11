#include "JWSystemRender.h"
#include "JWEntity.h"
#include "../Core/JWDX.h"
#include "../Core/JWCamera.h"

using namespace JWEngine;

JWSystemRender::~JWSystemRender()
{
	if (m_vpComponents.size())
	{
		for (auto& iter : m_vpComponents)
		{
			JW_DELETE(iter);
		}
	}
}

void JWSystemRender::CreateSystem(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept
{
	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;
	
	// Set base directory.
	m_BaseDirectory = BaseDirectory;
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

void JWSystemRender::Execute() noexcept
{
	for (auto& iter : m_vpComponents)
	{
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
}

void JWSystemRender::AnimateOnGPU(SComponentRender& Component) noexcept
{
	auto type = Component.RenderType;
	if (type == ERenderType::Model_Rigged)
	{
		auto& anim_state = Component.AnimationState;
		auto& model = Component.PtrModel;

		// Set VS texture
		m_pDX->GetDeviceContext()->VSSetShaderResources(0, 1, &Component.PtrAnimationTexture->TextureSRV);

		if (anim_state.CurrAnimationID > 0)
		{
			// Not TPose

			auto& current_anim{ model->RiggedModelData.AnimationSet.vAnimations[anim_state.CurrAnimationID - 1] };

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
}

void JWSystemRender::AnimateOnCPU(SComponentRender& Component) noexcept
{
	auto type = Component.RenderType;
	if (type == ERenderType::Model_Rigged)
	{
		auto& anim_state = Component.AnimationState;
		auto& model = Component.PtrModel;

		if ((Component.FlagRenderOption & JWFlagRenderOption_DrawTPose) || (anim_state.CurrAnimationID == 0))
		{
			// TPose

			anim_state.CurrFrameTime = 0.0f;
			anim_state.NextFrameTime = 0.0f;
			anim_state.TweeningTime = 0.0f;

			UpdateNodeTPoseIntoBones(anim_state.CurrAnimationTick, model->RiggedModelData, model->RiggedModelData.NodeTree.vNodes[0],
				XMMatrixIdentity());

			// Update bone's T-Pose transformation for shader's constant buffer
			for (size_t iterator_bone_mat{}; iterator_bone_mat < model->RiggedModelData.BoneTree.vBones.size(); ++iterator_bone_mat)
			{
				m_VSCBCPUAnimation.TransformedBoneMatrices[iterator_bone_mat] =
					XMMatrixTranspose(model->RiggedModelData.BoneTree.vBones[iterator_bone_mat].FinalTransformation);
			}
		}
		else
		{
			// Not TPose

			auto& current_anim{ model->RiggedModelData.AnimationSet.vAnimations[anim_state.CurrAnimationID - 1] };

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
				anim_state, model->RiggedModelData, model->RiggedModelData.NodeTree.vNodes[0], XMMatrixIdentity());

			// Update bone's final transformation for shader's constant buffer
			for (size_t iterator_bone_mat{}; iterator_bone_mat < model->RiggedModelData.BoneTree.vBones.size(); ++iterator_bone_mat)
			{
				m_VSCBCPUAnimation.TransformedBoneMatrices[iterator_bone_mat] =
					XMMatrixTranspose(model->RiggedModelData.BoneTree.vBones[iterator_bone_mat].FinalTransformation);
			}
		}
	}
}

PRIVATE void JWSystemRender::UpdateNodeAnimationIntoBones(bool UseInterpolation, SAnimationState& AnimationState, SRiggedModelData& ModelData,
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

PRIVATE void JWSystemRender::UpdateNodeTPoseIntoBones(float AnimationTime, SRiggedModelData& ModelData, const SModelNode& CurrentNode,
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
		m_pDX->UpdatePSCBFlags(
			(Component.FlagRenderOption & JWFlagRenderOption_UseTexture),
			(Component.FlagRenderOption & JWFlagRenderOption_UseLighting)
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
		break;
	case ERenderType::Model_Rigged:
		WVP = XMMatrixTranspose(component_world_matrix * m_pCamera->GetViewProjectionMatrix());
		World = XMMatrixTranspose(component_world_matrix);

		if (Component.FlagRenderOption & JWFlagRenderOption_UseGPUAnimation)
		{
			m_VSCBFlags.ShouldUseGPUAnimation = TRUE;
			
			m_pDX->UpdateVSCBGPUAnimation(m_VSCBGPUAnimation);
			m_pDX->UpdateVSCBFlags(m_VSCBFlags);
		}
		else
		{
			m_VSCBFlags.ShouldUseGPUAnimation = FALSE;
			
			m_pDX->UpdateVSCBCPUAnimation(m_VSCBCPUAnimation);
			m_pDX->UpdateVSCBFlags(m_VSCBFlags);
		}
		break;
	case ERenderType::Image_2D:
		WVP = XMMatrixTranspose(m_pCamera->GetTransformedOrthographicMatrix());
		break;
	case ERenderType::Model_Line3D:
		WVP = XMMatrixTranspose(component_world_matrix * m_pCamera->GetViewProjectionMatrix());
		break;
	case ERenderType::Model_Line2D:
		WVP = XMMatrixTranspose(component_world_matrix * m_pCamera->GetFixedOrthographicMatrix());
		break;
	default:
		break;
	}

	m_VSCBSpace.WVP = WVP;
	m_VSCBSpace.World = World;

	m_pDX->UpdateVSCBSpace(m_VSCBSpace);
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

	if ((type == ERenderType::Model_Static) || (type == ERenderType::Model_Rigged) || (type == ERenderType::Image_2D))
	{
		// Set IA primitive topology
		ptr_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else if ((type == ERenderType::Model_Line3D) || (type == ERenderType::Model_Line2D))
	{
		// Set IA primitive topology
		ptr_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
	
	switch (type)
	{
	case ERenderType::Model_Static:
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, &model->ModelVertexBuffer, model->StaticModelData.VertexData.GetPtrStride(), model->StaticModelData.VertexData.GetPtrOffset());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(model->ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(model->StaticModelData.IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Model_Rigged:
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, &model->ModelVertexBuffer, model->RiggedModelData.VertexData.GetPtrStride(), model->RiggedModelData.VertexData.GetPtrOffset());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(model->ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(model->RiggedModelData.IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Image_2D:
		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, &image->m_VertexBuffer, image->m_VertexData.GetPtrStride(), image->m_VertexData.GetPtrOffset());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(image->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(image->m_IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Model_Line3D:
		// Set IA primitive topology
		ptr_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, &line->m_VertexBuffer, line->m_VertexData.GetPtrStride(), line->m_VertexData.GetPtrOffset());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(line->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(line->m_IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Model_Line2D:
		// Set IA primitive topology
		ptr_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		// Set IA vertex buffer
		ptr_device_context->IASetVertexBuffers(
			0, 1, &line->m_VertexBuffer, line->m_VertexData.GetPtrStride(), line->m_VertexData.GetPtrOffset());

		// Set IA index buffer
		ptr_device_context->IASetIndexBuffer(line->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		ptr_device_context->DrawIndexed(line->m_IndexData.GetCount(), 0, 0);
		break;
	default:
		break;
	}

	// Draw normals
	if (Component.FlagRenderOption & JWFlagRenderOption_DrawNormals)
	{
		DrawNormals(Component);
	}
}

PRIVATE void JWSystemRender::DrawNormals(SComponentRender& Component) noexcept
{
	auto& model = Component.PtrModel;
	const auto& component_transform = Component.PtrEntity->GetComponentTransform();
	const auto& world_matrix = component_transform->WorldMatrix;

	assert(model);

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
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &model->NormalVertexBuffer,
		model->NormalData.VertexData.GetPtrStride(), model->NormalData.VertexData.GetPtrOffset());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(model->NormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(model->NormalData.IndexData.GetCount(), 0, 0);
}
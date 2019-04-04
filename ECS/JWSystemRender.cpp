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
	m_vpComponents[slot]->PtrDX = m_pDX;
	m_vpComponents[slot]->PtrCamera = m_pCamera;
	m_vpComponents[slot]->PtrBaseDirectory = &m_BaseDirectory;

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

void JWSystemRender::Update() noexcept
{
	for (auto& iter : m_vpComponents)
	{
		// Set depth stencil state for the component
		m_pDX->SetDepthStencilState(iter->DepthStencilState);

		Animate(*iter);

		SetShaders(*iter);

		Draw(*iter);
	}
}

void JWSystemRender::Animate(SComponentRender& Component) noexcept
{
	auto type = Component.RenderType;
	if (type == ERenderType::Model_RiggedModel)
	{
		auto& model = Component.Model;
		auto& current_animation_id = model.RiggedModelData.CurrentAnimationID;

		if (Component.FlagRenderOption & JWFlagRenderOption_DrawTPose)
		{
			UpdateNodeTPoseIntoBones(model.RiggedModelData.CurrentAnimationTick, model.RiggedModelData, model.RiggedModelData.NodeTree.vNodes[0],
				XMMatrixIdentity());

			// Update bone's T-Pose transformation for shader's constant buffer
			for (size_t iterator_bone_mat{}; iterator_bone_mat < model.RiggedModelData.BoneTree.vBones.size(); ++iterator_bone_mat)
			{
				m_VSCBRigged.TransformedBoneMatrices[iterator_bone_mat] =
					XMMatrixTranspose(model.RiggedModelData.BoneTree.vBones[iterator_bone_mat].FinalTransformation);
			}
		}
		else
		{
			if (current_animation_id != KSizeTInvalid)
			{
				auto& current_anim{ model.RiggedModelData.AnimationSet.vAnimations[current_animation_id] };

				// Reset tick if the animation is over.
				if (model.RiggedModelData.CurrentAnimationTick >= current_anim.TotalAnimationTicks)
				{
					model.RiggedModelData.CurrentAnimationTick = 0;

					if (!model.RiggedModelData.ShouldRepeatCurrentAnimation)
					{
						// Non-repeating animation
						current_animation_id = KSizeTInvalid;
					}
				}

				// Update bones' transformations for the animation.
				UpdateNodeAnimationIntoBones((Component.FlagRenderOption & JWFlagRenderOption_UseAnimationInterpolation),
					model.RiggedModelData.CurrentAnimationTick, model.RiggedModelData, model.RiggedModelData.NodeTree.vNodes[0], XMMatrixIdentity());

				// Update bone's final transformation for shader's constant buffer
				for (size_t iterator_bone_mat{}; iterator_bone_mat < model.RiggedModelData.BoneTree.vBones.size(); ++iterator_bone_mat)
				{
					m_VSCBRigged.TransformedBoneMatrices[iterator_bone_mat] =
						XMMatrixTranspose(model.RiggedModelData.BoneTree.vBones[iterator_bone_mat].FinalTransformation);
				}

				// Advance animation tick
				//model_data.CurrentAnimationTick += model_data.AnimationSet.vAnimations[current_animation_id].AnimationTicksPerGameTick;
				model.RiggedModelData.CurrentAnimationTick += 1.0f;
			}
			else
			{
				// No animation is set.
			}
		}
	}
}

PRIVATE void JWSystemRender::UpdateNodeAnimationIntoBones(bool UseInterpolation, float AnimationTime, SRiggedModelData& ModelData,
	const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept
{
	XMMATRIX global_transformation = CurrentNode.Transformation * Accumulated;

	if (CurrentNode.BoneID >= 0)
	{
		auto& bone = ModelData.BoneTree.vBones[CurrentNode.BoneID];
		auto& current_animation = ModelData.AnimationSet.vAnimations[ModelData.CurrentAnimationID];

		// Calculate current animation time for interpolation
		float CurrAnimationTime = AnimationTime - fmodf(AnimationTime, current_animation.AnimationTicksPerGameTick);

		// Calculate next animation time for interpolation
		float NextAnimationTime = CurrAnimationTime + current_animation.AnimationTicksPerGameTick;

		// Interpolation factor Delta's range is [0.0, 1.0]
		float Delta = (AnimationTime - CurrAnimationTime) / current_animation.AnimationTicksPerGameTick;

		if (NextAnimationTime > current_animation.TotalAnimationTicks)
		{
			NextAnimationTime = 0;
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
					if (key.TimeInTicks <= CurrAnimationTime)
					{
						scaling_key_a = XMLoadFloat3(&key.Key);
					}
					if (key.TimeInTicks <= NextAnimationTime)
					{
						scaling_key_b = XMLoadFloat3(&key.Key);
					}
				}
				scaling_interpolated = scaling_key_a + (Delta * (scaling_key_b - scaling_key_a));

				// #2. Find rotation keys
				for (const auto& key : CurrentNode_animation.vKeyRotation)
				{
					if (key.TimeInTicks <= CurrAnimationTime)
					{
						rotation_key_a = key.Key;
					}
					if (key.TimeInTicks <= NextAnimationTime)
					{
						rotation_key_b = key.Key;
					}
				}
				rotation_interpolated = XMQuaternionSlerp(rotation_key_a, rotation_key_b, Delta);

				// #3. Find translation keys
				for (const auto& key : CurrentNode_animation.vKeyPosition)
				{
					if (key.TimeInTicks <= CurrAnimationTime)
					{
						translation_key_a = XMLoadFloat3(&key.Key);
					}
					if (key.TimeInTicks <= NextAnimationTime)
					{
						translation_key_b = XMLoadFloat3(&key.Key);
					}
				}
				translation_interpolated = translation_key_a + (Delta * (translation_key_b - translation_key_a));

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
			UpdateNodeAnimationIntoBones(UseInterpolation, AnimationTime, ModelData, ModelData.NodeTree.vNodes[child_id], global_transformation);
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
	auto type = Component.RenderType;
	const auto& model = Component.Model;

	const auto& component_transform = Component.PtrEntity->GetComponentTransform();
	const auto& world_matrix = component_transform->WorldMatrix;

	// Set PS
	m_pDX->SetPS(Component.PixelShader);

	if (Component.PixelShader == EPixelShader::PSBase)
	{
		// Update PS constant buffer
		m_pDX->UpdatePSCBFlags(
			(Component.FlagRenderOption & JWFlagRenderOption_UseTexture),
			(Component.FlagRenderOption & JWFlagRenderOption_UseLighting)
		);
	}

	// Set PS texture
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &model.TextureShaderResourceView);

	// Set PS sampler
	m_pDX->SetPSSamplerState(ESamplerState::MinMagMipLinearWrap);

	// Set VS
	m_pDX->SetVS(Component.VertexShader);

	// Update VS constant buffer
	switch (type)
	{
	case ERenderType::Model_StaticModel:
		m_VSCBStatic.WVP = XMMatrixTranspose(world_matrix * m_pCamera->GetViewProjectionMatrix());
		m_VSCBStatic.World = XMMatrixTranspose(world_matrix);
		m_pDX->UpdateVSCBStatic(m_VSCBStatic);
		break;

	case ERenderType::Model_RiggedModel:
		m_VSCBRigged.WVP = XMMatrixTranspose(world_matrix * m_pCamera->GetViewProjectionMatrix());
		m_VSCBRigged.World = XMMatrixTranspose(world_matrix);
		m_pDX->UpdateVSCBRigged(m_VSCBRigged);
		break;

	case ERenderType::Image_2D:
		m_VSCBStatic.WVP = XMMatrixIdentity() * m_pCamera->GetTransformedOrthographicMatrix();
		m_pDX->UpdateVSCBStatic(m_VSCBStatic);

	default:
		break;
	}
}

PRIVATE void JWSystemRender::Draw(SComponentRender& Component) noexcept
{
	auto type = Component.RenderType;
	auto& model = Component.Model;
	auto& image = Component.Image;

	if (Component.FlagRenderOption & JWFlagRenderOption_UseTransparency)
	{
		m_pDX->SetBlendState(EBlendState::Transprent);
	}
	else
	{
		m_pDX->SetBlendState(EBlendState::Opaque);
	}
	
	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	switch (type)
	{
	case ERenderType::Model_StaticModel:
		// Set IA vertex buffer
		m_pDX->GetDeviceContext()->IASetVertexBuffers(
			0, 1, &model.ModelVertexBuffer, model.StaticModelData.VertexData.GetPtrStride(), model.StaticModelData.VertexData.GetPtrOffset());

		// Set IA index buffer
		m_pDX->GetDeviceContext()->IASetIndexBuffer(model.ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		m_pDX->GetDeviceContext()->DrawIndexed(model.StaticModelData.IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Model_RiggedModel:
		// Set IA vertex buffer
		m_pDX->GetDeviceContext()->IASetVertexBuffers(
			0, 1, &model.ModelVertexBuffer, model.RiggedModelData.VertexData.GetPtrStride(), model.RiggedModelData.VertexData.GetPtrOffset());

		// Set IA index buffer
		m_pDX->GetDeviceContext()->IASetIndexBuffer(model.ModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		m_pDX->GetDeviceContext()->DrawIndexed(model.RiggedModelData.IndexData.GetCount(), 0, 0);
		break;
	case ERenderType::Image_2D:
		// Set IA vertex buffer
		m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &image.m_VertexBuffer, image.m_VertexData.GetPtrStride(), image.m_VertexData.GetPtrOffset());

		// Set IA index buffer
		m_pDX->GetDeviceContext()->IASetIndexBuffer(image.m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw indexed
		m_pDX->GetDeviceContext()->DrawIndexed(image.m_IndexData.GetCount(), 0, 0);
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
	auto& model = Component.Model;
	const auto& component_transform = Component.PtrEntity->GetComponentTransform();
	const auto& world_matrix = component_transform->WorldMatrix;

	// Set VS Base
	m_pDX->SetVS(EVertexShader::VSBase);

	// Update VS constant buffer
	m_VSCBStatic.WVP = XMMatrixTranspose(world_matrix * m_pCamera->GetViewProjectionMatrix());
	m_VSCBStatic.World = XMMatrixTranspose(world_matrix);
	m_pDX->UpdateVSCBStatic(m_VSCBStatic);

	// Set PS Base
	m_pDX->SetPS(EPixelShader::PSBase);

	// Update PS constant buffer
	m_pDX->UpdatePSCBFlags(false, false);

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &model.NormalVertexBuffer,
		model.NormalData.VertexData.GetPtrStride(), model.NormalData.VertexData.GetPtrOffset());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(model.NormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(model.NormalData.IndexData.GetCount(), 0, 0);
}
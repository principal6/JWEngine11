#include "JWModel.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

JWModel::~JWModel()
{
	JW_RELEASE(m_TextureShaderResourceView);
	
	JW_RELEASE(m_NormalVertexBuffer);
	JW_RELEASE(m_NormalIndexBuffer);

	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWModel::Create(JWDX& DX, JWCamera& Camera) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	// Set JWDX pointer.
	m_pDX = &DX;

	// Set JWCamera pointer.
	m_pCamera = &Camera;

	m_MatrixWorld = XMMatrixIdentity();
	m_MatrixTranslation = XMMatrixIdentity();
	m_MatrixRotation = XMMatrixIdentity();
	m_MatrixScale = XMMatrixIdentity();

	m_IsValid = true;
}

void JWModel::SetStaticModelData(SStaticModelData ModelData) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsModelLoaded);

	CheckValidity();

	m_ModelType = EModelType::StaticModel;

	// Save the model data
	m_StaticModelData = ModelData;

	CreateModelVertexIndexBuffers();

	// Create texture if there is
	if (ModelData.HasTexture)
	{
		m_HasTexture = true;
		CreateTexture(ModelData.TextureFileNameW);
	}

	// For normal line drawing
	size_t iterator_vertex{};
	XMFLOAT3 second_vertex_position{};
	for (const auto& vertex : ModelData.VertexData.Vertices)
	{
		second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
		second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
		second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

		NormalAddVertex(SStaticVertex(vertex.Position, KDefaultColorNormals));
		NormalAddVertex(SStaticVertex(second_vertex_position, KDefaultColorNormals));
		NormalAddIndex(SIndex2(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
		++iterator_vertex;
	}
	NormalAddEnd();

	m_IsModelLoaded = true;
}

void JWModel::SetSkinnedModelData(SSkinnedModelData ModelData) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsModelLoaded);

	CheckValidity();

	m_ModelType = EModelType::SkinnedModel;

	// Save the model data
	m_SkinnedModelData = ModelData;

	CreateModelVertexIndexBuffers();

	// Create texture if there is
	if (ModelData.HasTexture)
	{
		m_HasTexture = true;
		CreateTexture(ModelData.TextureFileNameW);
	}

	// For normal line drawing
	size_t iterator_vertex{};
	XMFLOAT3 second_vertex_position{};
	for (const auto& vertex : ModelData.VertexData.Vertices)
	{
		second_vertex_position.x = vertex.Position.x + vertex.Normal.x;
		second_vertex_position.y = vertex.Position.y + vertex.Normal.y;
		second_vertex_position.z = vertex.Position.z + vertex.Normal.z;

		NormalAddVertex(SStaticVertex(vertex.Position, KDefaultColorNormals));
		NormalAddVertex(SStaticVertex(second_vertex_position, KDefaultColorNormals));
		NormalAddIndex(SIndex2(static_cast<UINT>(iterator_vertex * 2), static_cast<UINT>(iterator_vertex * 2 + 1)));
		++iterator_vertex;
	}
	NormalAddEnd();

	m_IsModelLoaded = true;
}

void JWModel::SetModel2Data(SModel2Data Model2Data) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsMode2lLoaded);

	CheckValidity();

	m_ModelType = EModelType::LineModel;

	m_NormalData = Model2Data;
	NormalAddEnd();

	m_IsMode2lLoaded = true;
}

PRIVATE void JWModel::CreateTexture(WSTRING TextureFileName) noexcept
{
	if (TextureFileName.find(L".dds") != std::string::npos)
	{
		CreateDDSTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &m_TextureShaderResourceView, 0);
	}
	else
	{
		CreateWICTextureFromFile(m_pDX->GetDevice(), TextureFileName.c_str(), nullptr, &m_TextureShaderResourceView, 0);
	}
}

PRIVATE void JWModel::CheckValidity() const noexcept
{
	if (!m_IsValid)
	{
		JWAbort("JWModel object not valid. You must call JWModel::Create() first");
	}
}

PRIVATE void JWModel::CreateModelVertexIndexBuffers() noexcept
{
	switch (m_ModelType)
	{
	case JWEngine::EModelType::StaticModel:
		// Create vertex buffer
		m_pDX->CreateStaticVertexBuffer(m_StaticModelData.VertexData.GetByteSize(), m_StaticModelData.VertexData.GetPtrData(), &m_VertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(m_StaticModelData.IndexData.GetByteSize(), m_StaticModelData.IndexData.GetPtrData(), &m_IndexBuffer);

		break;
	case JWEngine::EModelType::SkinnedModel:
		// Create vertex buffer
		m_pDX->CreateStaticVertexBuffer(m_SkinnedModelData.VertexData.GetByteSize(), m_SkinnedModelData.VertexData.GetPtrData(), &m_VertexBuffer);

		// Create index buffer
		m_pDX->CreateIndexBuffer(m_SkinnedModelData.IndexData.GetByteSize(), m_SkinnedModelData.IndexData.GetPtrData(), &m_IndexBuffer);

		break;
	default:
		break;
	}
}

PRIVATE auto JWModel::NormalAddVertex(const SStaticVertex& Vertex) noexcept->JWModel&
{
	m_NormalData.VertexData.Vertices.push_back(Vertex);

	return *this;
}

PRIVATE auto JWModel::NormalAddIndex(const SIndex2& Index) noexcept->JWModel&
{
	m_NormalData.IndexData.Indices.push_back(Index);

	return *this;
}

PRIVATE void JWModel::NormalAddEnd() noexcept
{
	// Create vertex buffer
	m_pDX->CreateStaticVertexBuffer(m_NormalData.VertexData.GetByteSize(), m_NormalData.VertexData.GetPtrData(), &m_NormalVertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_NormalData.IndexData.GetByteSize(), m_NormalData.IndexData.GetPtrData(), &m_NormalIndexBuffer);
}

auto JWModel::SetWorldMatrixToIdentity() noexcept->JWModel&
{
	m_MatrixWorld = XMMatrixIdentity();

	return *this;
}

auto JWModel::SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder Order) noexcept->JWModel&
{
	m_CalculationOrder = Order;

	UpdateWorldMatrix();

	return *this;
}

auto JWModel::SetTranslation(XMFLOAT3 Offset) noexcept->JWModel&
{
	m_MatrixTranslation = XMMatrixTranslation(Offset.x, Offset.y, Offset.z);

	UpdateWorldMatrix();

	return *this;
}

auto JWModel::SetRotation(XMFLOAT4 RotationAxis, float Angle) noexcept->JWModel&
{
	XMVECTOR rotation_axis = XMVectorSet(RotationAxis.x, RotationAxis.y, RotationAxis.z, RotationAxis.w);
	m_MatrixRotation = XMMatrixRotationAxis(rotation_axis, Angle);

	UpdateWorldMatrix();

	return *this;
}

auto JWModel::SetScale(XMFLOAT3 Scale) noexcept->JWModel&
{
	m_MatrixScale = XMMatrixScaling(Scale.x, Scale.y, Scale.z);

	UpdateWorldMatrix();

	return *this;
}

PRIVATE void JWModel::UpdateWorldMatrix() noexcept
{
	switch (m_CalculationOrder)
	{
	case JWEngine::EWorldMatrixCalculationOrder::TransRotScale:
		m_MatrixWorld = m_MatrixTranslation * m_MatrixRotation * m_MatrixScale;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::TransScaleRot:
		m_MatrixWorld = m_MatrixTranslation * m_MatrixScale * m_MatrixRotation;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::RotTransScale:
		m_MatrixWorld = m_MatrixRotation * m_MatrixTranslation * m_MatrixScale;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::RotScaleTrans:
		m_MatrixWorld = m_MatrixRotation * m_MatrixScale * m_MatrixTranslation;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::ScaleTransRot:
		m_MatrixWorld = m_MatrixScale * m_MatrixTranslation * m_MatrixRotation;
		break;
	case JWEngine::EWorldMatrixCalculationOrder::ScaleRotTrans:
		m_MatrixWorld = m_MatrixScale * m_MatrixRotation * m_MatrixTranslation;
		break;
	default:
		break;
	}

	// Update world position of the model
	m_WorldPosition = XMVector3TransformCoord(XMVectorZero(), m_MatrixWorld);
}

auto JWModel::SetAnimation(size_t AnimationID, bool ShouldRepeat) noexcept->JWModel&
{
	if (m_ModelType == EModelType::SkinnedModel)
	{
		if (m_SkinnedModelData.AnimationSet.vAnimations.size())
		{
			AnimationID = min(AnimationID, m_SkinnedModelData.AnimationSet.vAnimations.size() - 1);

			if (m_SkinnedModelData.CurrentAnimationID != AnimationID)
			{
				m_SkinnedModelData.CurrentAnimationID = AnimationID;
				m_SkinnedModelData.CurrentAnimationTick = 0;
				m_SkinnedModelData.ShouldRepeatCurrentAnimation = ShouldRepeat;
			}
		}
		else
		{
			// Model has no animation set
			JWAbort("This model doesn't have any animation set.");
		}
	}

	return *this;
}

auto JWModel::Animate() noexcept->JWModel&
{
	if (m_ModelType == EModelType::SkinnedModel)
	{
		auto& current_animation_id = m_SkinnedModelData.CurrentAnimationID;

		if (current_animation_id != KSizeTInvalid)
		{
			auto& current_anim{ m_SkinnedModelData.AnimationSet.vAnimations[current_animation_id] };

			// Reset tick if the animation is over.
			if (m_SkinnedModelData.CurrentAnimationTick >= current_anim.TotalAnimationTicks)
			{
				m_SkinnedModelData.CurrentAnimationTick = 0;

				if (!m_SkinnedModelData.ShouldRepeatCurrentAnimation)
				{
					// Non-repeating animation
					current_animation_id = KSizeTInvalid;
				}
			}

			// Update bones' transformations for the animation.
			UpdateBonesTransformation();

			// Advance animation tick
			//m_SkinnedModelData.CurrentAnimationTick += m_SkinnedModelData.AnimationSet.vAnimations[current_animation_id].AnimationTicksPerGameTick;
			m_SkinnedModelData.CurrentAnimationTick += 1.0f;
		}
		else
		{
			// No animation is set.
		}
	}

	return *this;
}

auto JWModel::SetTPose() noexcept->JWModel&
{
	if (m_ModelType == EModelType::SkinnedModel)
	{
		UpdateNodeTPoseIntoBones(m_SkinnedModelData.CurrentAnimationTick, m_SkinnedModelData.NodeTree.vNodes[0], XMMatrixIdentity());

		// Update bone's T-Pose transformation for shader's constant buffer
		for (size_t iterator_bone_mat{}; iterator_bone_mat < m_SkinnedModelData.BoneTree.vBones.size(); ++iterator_bone_mat)
		{
			m_VSCBSkinned.TransformedBoneMatrices[iterator_bone_mat] =
				XMMatrixTranspose(m_SkinnedModelData.BoneTree.vBones[iterator_bone_mat].FinalTransformation);
		}
	}

	return *this;
}

PRIVATE void JWModel::UpdateBonesTransformation() noexcept
{
	UpdateNodeAnimationIntoBones(m_SkinnedModelData.CurrentAnimationTick, m_SkinnedModelData.NodeTree.vNodes[0], XMMatrixIdentity());

	// Update bone's final transformation for shader's constant buffer
	for (size_t iterator_bone_mat{}; iterator_bone_mat < m_SkinnedModelData.BoneTree.vBones.size(); ++iterator_bone_mat)
	{
		m_VSCBSkinned.TransformedBoneMatrices[iterator_bone_mat] =
			XMMatrixTranspose(m_SkinnedModelData.BoneTree.vBones[iterator_bone_mat].FinalTransformation);
	}
}

PRIVATE void JWModel::UpdateNodeAnimationIntoBones(float AnimationTime, const SModelNode& node, const XMMATRIX Accumulated) noexcept
{
	XMMATRIX global_transformation = node.Transformation * Accumulated;

	if (node.BoneID >= 0)
	{
		auto& bone = m_SkinnedModelData.BoneTree.vBones[node.BoneID];
		auto& current_animation = m_SkinnedModelData.AnimationSet.vAnimations[m_SkinnedModelData.CurrentAnimationID];

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

		for (const auto& node_animation : current_animation.vNodeAnimation)
		{
			if (node_animation.NodeID == node.ID)
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
				for (const auto& key : node_animation.vKeyScaling)
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
				if (!m_ShouldInterpolateAnimation)
				{
					scaling_interpolated = scaling_key_a;
				}
				matrix_scaling = XMMatrixScalingFromVector(scaling_interpolated);

				// #2. Find rotation keys
				for (const auto& key : node_animation.vKeyRotation)
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
				if (!m_ShouldInterpolateAnimation)
				{
					rotation_interpolated = rotation_key_a;
				}
				matrix_rotation = XMMatrixRotationQuaternion(rotation_interpolated);

				// #3. Find translation keys
				for (const auto& key : node_animation.vKeyPosition)
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
				if (!m_ShouldInterpolateAnimation)
				{
					translation_interpolated = translation_key_a;
				}
				matrix_translation = XMMatrixTranslationFromVector(translation_interpolated);

				global_transformation = matrix_scaling * matrix_rotation * matrix_translation * Accumulated;

				break;
			}
		}

		bone.FinalTransformation = bone.Offset * global_transformation;
	}

	if (node.vChildrenID.size())
	{
		for (auto child_id : node.vChildrenID)
		{
			UpdateNodeAnimationIntoBones(AnimationTime, m_SkinnedModelData.NodeTree.vNodes[child_id], global_transformation);
		}
	}
}

PRIVATE void JWModel::UpdateNodeTPoseIntoBones(float AnimationTime, const SModelNode& node, const XMMATRIX Accumulated) noexcept
{
	XMMATRIX accumulation = node.Transformation * Accumulated;

	if (node.BoneID >= 0)
	{
		auto& bone = m_SkinnedModelData.BoneTree.vBones[node.BoneID];

		bone.FinalTransformation = bone.Offset * accumulation;
	}

	if (node.vChildrenID.size())
	{
		for (auto child_id : node.vChildrenID)
		{
			UpdateNodeTPoseIntoBones(AnimationTime, m_SkinnedModelData.NodeTree.vNodes[child_id], accumulation);
		}
	}
}

auto JWModel::ShouldDrawNormals(bool Value) noexcept->JWModel&
{
	m_ShouldDrawNormals = Value;

	return *this;
}

auto JWModel::ShouldBeLit(bool Value) noexcept->JWModel&
{
	m_ShouldBeLit = Value;

	return *this;
}

auto JWModel::ShouldInterpolateAnimation(bool Value) noexcept->JWModel&
{
	m_ShouldInterpolateAnimation = Value;

	return *this;
}

auto JWModel::GetWorldPosition() noexcept->XMVECTOR
{
	return m_WorldPosition;
}

auto JWModel::GetDistanceFromCamera() noexcept->float
{
	float distance_x = XMVectorGetX(m_WorldPosition) - XMVectorGetX(m_pCamera->GetPosition());
	float distance_y = XMVectorGetY(m_WorldPosition) - XMVectorGetY(m_pCamera->GetPosition());
	float distance_z = XMVectorGetZ(m_WorldPosition) - XMVectorGetZ(m_pCamera->GetPosition());

	return (distance_x * distance_x + distance_y * distance_y + distance_z * distance_z);
}

PRIVATE void JWModel::Update() noexcept
{
	// Enable Z-buffer for 3D drawing
	m_pDX->SetDepthStencilState(EDepthStencilState::ZEnabled);

	// Set VS
	switch (m_ModelType)
	{
	case JWEngine::EModelType::StaticModel:
		m_pDX->SetVSBase();

		// Set VS constant buffer
		m_VSCBStatic.WVP = XMMatrixTranspose(m_MatrixWorld * m_pCamera->GetViewProjectionMatrix());
		m_VSCBStatic.World = XMMatrixTranspose(m_MatrixWorld);
		m_pDX->SetVSCBStatic(m_VSCBStatic);

		break;
	case JWEngine::EModelType::SkinnedModel:
		m_pDX->SetVSAnim();

		// Set VS constant buffer
		m_VSCBSkinned.WVP = XMMatrixTranspose(m_MatrixWorld * m_pCamera->GetViewProjectionMatrix());
		m_VSCBSkinned.World = XMMatrixTranspose(m_MatrixWorld);

		m_pDX->SetVSCBSkinned(m_VSCBSkinned);

		break;
	default:
		break;
	}

	// Set PS
	m_pDX->SetPSBase();

	// Set PS constant buffer
	m_pDX->SetPSCBDefaultFlags(m_HasTexture, m_ShouldBeLit);

	// Set PS texture and sampler
	m_pDX->GetDeviceContext()->PSSetShaderResources(0, 1, &m_TextureShaderResourceView);
	m_pDX->SetPSSamplerState(ESamplerState::MinMagMipLinearWrap);
}

void JWModel::Draw() noexcept
{
	Update();

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set IA vertex buffer
	switch (m_ModelType)
	{
	case JWEngine::EModelType::StaticModel:
		m_pDX->GetDeviceContext()->IASetVertexBuffers(
			0, 1, &m_VertexBuffer, m_StaticModelData.VertexData.GetPtrStride(), m_StaticModelData.VertexData.GetPtrOffset());

		break;
	case JWEngine::EModelType::SkinnedModel:
		m_pDX->GetDeviceContext()->IASetVertexBuffers(
			0, 1, &m_VertexBuffer, m_SkinnedModelData.VertexData.GetPtrStride(), m_SkinnedModelData.VertexData.GetPtrOffset());

		break;
	default:
		break;
	}
	
	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	switch (m_ModelType)
	{
	case JWEngine::EModelType::StaticModel:
		m_pDX->GetDeviceContext()->DrawIndexed(m_StaticModelData.IndexData.GetCount(), 0, 0);

		break;
	case JWEngine::EModelType::SkinnedModel:
		m_pDX->GetDeviceContext()->DrawIndexed(m_SkinnedModelData.IndexData.GetCount(), 0, 0);

		break;
	default:
		break;
	}
	
	// Draw normals
	if ((m_ShouldDrawNormals) || (m_IsMode2lLoaded))
	{
		DrawNormals();
	}
}

PRIVATE void JWModel::DrawNormals() noexcept
{
	// Set VS base
	m_pDX->SetVSBase();

	// Set VS constant buffer
	m_VSCBStatic.WVP = XMMatrixTranspose(m_MatrixWorld * m_pCamera->GetViewProjectionMatrix());
	m_VSCBStatic.World = XMMatrixTranspose(m_MatrixWorld);

	// Set PS constant buffer
	m_pDX->SetPSCBDefaultFlags(false, false);

	// Set IA primitive topology
	m_pDX->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Set IA vertex buffer
	m_pDX->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_NormalVertexBuffer, m_NormalData.VertexData.GetPtrStride(), m_NormalData.VertexData.GetPtrOffset());

	// Set IA index buffer
	m_pDX->GetDeviceContext()->IASetIndexBuffer(m_NormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Draw
	m_pDX->GetDeviceContext()->DrawIndexed(m_NormalData.IndexData.GetCount(), 0, 0);
}
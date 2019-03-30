#pragma once

#include "JWCommon.h"
#include "JWAssimpLoader.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;

	class JWModel
	{
	public:
		JWModel() = default;
		~JWModel();

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera) noexcept;

		// Called in JWGame class
		void SetStaticModelData(SStaticModelData ModelData) noexcept;
		void SetSkinnedModelData(SSkinnedModelData ModelData) noexcept;

		void SetModel2Data(SModel2Data Model2Data) noexcept;
		
		auto SetWorldMatrixToIdentity() noexcept->JWModel&;
		auto SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder Order) noexcept->JWModel&;
		auto SetTranslation(XMFLOAT3 Offset) noexcept->JWModel&;
		auto SetRotation(XMFLOAT4 RotationAxis, float Angle) noexcept->JWModel&;
		auto SetScale(XMFLOAT3 Scale) noexcept->JWModel&;

		// Animation
		auto SetAnimation(size_t AnimationID, bool ShouldRepeat = true) noexcept->JWModel&;
		auto Animate() noexcept->JWModel&;
		auto SetTPose() noexcept->JWModel&;
		
		auto ShouldDrawNormals(bool Value) noexcept->JWModel&;
		auto ShouldBeLit(bool Value) noexcept->JWModel&;
		auto ShouldInterpolateAnimation(bool Value) noexcept->JWModel&;

		auto GetWorldPosition() noexcept->XMVECTOR;
		auto GetDistanceFromCamera() noexcept->float;

		// Called in JWGame class
		void Draw() noexcept;

	private:
		void CheckValidity() const noexcept;

		void CreateTexture(WSTRING TextureFileName) noexcept;

		void CreateModelVertexIndexBuffers() noexcept;

		auto NormalAddVertex(const SStaticVertex& Vertex) noexcept->JWModel&;
		auto NormalAddIndex(const SIndex2& Index) noexcept->JWModel&;
		void NormalAddEnd() noexcept;

		// Called by Draw()
		void DrawNormals() noexcept;

		// Called by Draw()
		void Update() noexcept;

		void UpdateWorldMatrix() noexcept;

		// Called by Animate()
		void UpdateBonesTransformation() noexcept;

		// Called by UpdateBonesTransformation() and by this function itself (recursively)
		void UpdateNodeAnimationIntoBones(float AnimationTime, const SModelNode& node, const XMMATRIX Accumulated) noexcept;

		// Called by SetTPose()
		void UpdateNodeTPoseIntoBones(float AnimationTime, const SModelNode& node, const XMMATRIX Accumulated) noexcept;

	private:
		bool m_IsValid{ false };
		bool m_IsModelLoaded{ false };
		bool m_IsMode2lLoaded{ false };
		bool m_HasTexture{ false };
		bool m_ShouldDrawNormals{ false };
		bool m_ShouldBeLit{ true };
		bool m_ShouldInterpolateAnimation{ true };

		EModelType m_ModelType{ EModelType::Invalid };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};

		SStaticModelData m_StaticModelData{};
		SSkinnedModelData m_SkinnedModelData{};

		ID3D11Buffer* m_NormalVertexBuffer{};
		ID3D11Buffer* m_NormalIndexBuffer{};

		SModel2Data m_NormalData{};
		//SVertexData m_NormalVertexData{};
		//SIndex2Data m_NormalIndexData{};

		ID3D11ShaderResourceView* m_TextureShaderResourceView{};

		XMMATRIX m_MatrixWorld{};
		XMMATRIX m_MatrixTranslation{};
		XMMATRIX m_MatrixRotation{};
		XMMATRIX m_MatrixScale{};

		XMVECTOR m_WorldPosition{};

		EWorldMatrixCalculationOrder m_CalculationOrder{ EWorldMatrixCalculationOrder::ScaleRotTrans };

		SVSCBStatic m_VSCBStatic{};
		SVSCBSkinned m_VSCBSkinned{};
	};
};

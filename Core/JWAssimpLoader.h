#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	inline auto ConvertaiColor4DToXMFLOAT4(aiColor4D input)->XMFLOAT4
	{
		return XMFLOAT4(input.r, input.g, input.b, input.a);
	}

	inline auto ConvertaiVector3DToXMFLOAT2(aiVector3D input)->XMFLOAT2
	{
		return XMFLOAT2(input.x, input.y);
	}

	inline auto ConvertaiVector3DToXMFLOAT3(aiVector3D input)->XMFLOAT3
	{
		return XMFLOAT3(input.x, input.y, input.z);
	}

	inline auto ConvertaiVector3DToXMVECTORTexCoord(aiVector3D input)->XMVECTOR
	{
		return XMVectorSet(input.x, input.y, 0, 0);
	}

	inline auto ConvertaiVector3DToXMVECTORCoord(aiVector3D input)->XMVECTOR
	{
		return XMVectorSet(input.x, input.y, input.z, 1.0f);
	}

	inline auto ConvertaiVector3DToXMVECTORNormal(aiVector3D input)->XMVECTOR
	{
		return XMVectorSet(input.x, input.y, input.z, 0.0f);
	}
	
	inline auto ConvertaiMatrix4x4ToXMMATRIX(const aiMatrix4x4& input)->XMMATRIX
	{
		return XMMatrixTranspose(XMMatrixSet(input.a1, input.a2, input.a3, input.a4,
			input.b1, input.b2, input.b3, input.b4,
			input.c1, input.c2, input.c3, input.c4,
			input.d1, input.d2, input.d3, input.d4));
	}

	inline auto ConvertaiQuaternionToXMVECTOR(aiQuaternion input)->XMVECTOR
	{
		return XMVectorSet(input.x, input.y, input.z, input.w);
	}

	class JWAssimpLoader final
	{
	public:
		auto LoadNonRiggedModel(STRING Directory, STRING ModelFileName) noexcept->SModelData;

		auto LoadRiggedModel(STRING Directory, STRING ModelFileName) noexcept->SModelData;

		void LoadAdditionalAnimationIntoRiggedModel(SModelData& ModelData, STRING Directory, STRING ModelFileName) noexcept;

	private:
		void ExtractNodeTree(const aiScene* Scene, const aiNode* Node, int ParentNodeID, SModelNodeTree& OutNodeTree) noexcept;

		void BuildMeshesAndBonesFromNodes(const STRING Directory, const aiScene* Scene, SModelData& OutModelData) noexcept;

		void ExtractBone(const aiBone* paiBone, int VertexOffset, SModelBoneTree& OutBoneTree) noexcept;

		void MatchBonesAndVertices(const SModelBoneTree& BoneTree, SVertexDataModel& OutVertexData) noexcept;

		void MatchBonesAndNodes(const SModelBoneTree& BoneTree, SModelNodeTree& OutNodeTree) noexcept;

		void ExtractAnimationSet(const aiScene* Scene, const SModelNodeTree& NodeTree, SModelAnimationSet& OutAnimationSet);
	};
};
#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	struct SModelNode
	{
		// Current node's index in SModelNodeTree.vNodes
		int ID{};

		// Parent node's index in SModelNodeTree.vNodes
		int ParentID{ -1 };

		// Child nodes' index in SModelNodeTree.vNodes
		VECTOR<int> vChildrenID;

		STRING Name;
		XMMATRIX Transformation{};
		VECTOR<int> vMeshesID;

		// This data will be used to match bones to each vertex!
		// This is required, because different from nodes,
		// bones will be stored unordered in bone tree
		// so, if nodes don't have vertex indicator,
		// bones can't find vertex offset properly.
		int AccumulatedVerticesCount{};

		// BoneID is '-1' when this node is not referring to any bone.
		// If (BoneID >= 0) : this node matches the bone in model's bone tree
		int BoneID{ -1 };

		SModelNode() = default;
	};

	struct SModelNodeTree
	{
		// Each node will be stored in this vector.
		// Root node is always at index 0.
		VECTOR<SModelNode> vNodes;
		int TotalVerticesCount{};
	};

	struct SModelWeight
	{
		int VertexID{};
		float Weight{};

		SModelWeight() = default;
		SModelWeight(int _VertexID, float _Weight) :VertexID{ _VertexID }, Weight{ _Weight } {};
	};

	struct SModelBone
	{
		// Current bone's index in SModelBoneTree.vBones
		int ID;
		STRING Name;
		VECTOR<SModelWeight> vWeights;

		// Loaded at model import time, and will not be altered.
		XMMATRIX Offset{};

		// Updated evrey (rendering) frame according to node's animation.
		XMMATRIX FinalTransformation{};

		SModelBone() = default;
		SModelBone(STRING _Name) :Name{ _Name } {};
	};

	struct SModelBoneTree
	{
		VECTOR<SModelBone> vBones;
	};

	struct SModelAnimationKeyPosition
	{
		float TimeInTicks{};
		XMFLOAT3 Key{};
	};

	struct SModelAnimationKeyRotation
	{
		float TimeInTicks{};
		XMVECTOR Key{};
	};

	struct SModelAnimationKeyScaling
	{
		float TimeInTicks{};
		XMFLOAT3 Key{};
	};

	struct SModelNodeAnimation
	{
		size_t NodeID;
		VECTOR<SModelAnimationKeyPosition> vKeyPosition;
		VECTOR<SModelAnimationKeyRotation> vKeyRotation;
		VECTOR<SModelAnimationKeyScaling> vKeyScaling;
	};

	struct SModelAnimation
	{
		// Animation's name. This can be null.
		STRING Name;

		// Animation duration (= TotalAnimationTicks / AnimationTicksPerGameTick)
		int TotalFrameCount{};

		// Animation duration based on animation ticks
		float TotalAnimationTicks{};

		// Model renderer's crieterion of animation ticks per model renderer's second of time.
		float AnimationTicksPerSecond{};

		// Our game loop based animation time
		// In every tick of the main loop,
		// the animation time will be increased by this amount.
		float AnimationTicksPerGameTick{};

		VECTOR<SModelNodeAnimation> vNodeAnimation;

		SModelAnimation() = default;
	};

	struct SModelAnimationSet
	{
		VECTOR<SModelAnimation> vAnimations;
	};

	struct SStaticModelData
	{
		SStaticModelVertexData VertexData{};
		SModelIndexData IndexData{};

		bool HasTexture{ false };
		WSTRING TextureFileNameW{};
	};

	struct SRiggedModelData
	{
		SRiggedModelVertexData VertexData{};
		SModelIndexData IndexData{};

		bool HasTexture{ false };
		WSTRING TextureFileNameW{};

		// If no animation is set, CurrentAnimationID is KSizeTInvalid(-1)
		size_t CurrentAnimationID{ KSizeTInvalid };

		// If SetAnimation() is called, CurrentAnimationTick is reset to 0.
		float CurrentAnimationTick{};

		bool ShouldRepeatCurrentAnimation{ true };

		SModelNodeTree NodeTree{};
		SModelBoneTree BoneTree{};
		SModelAnimationSet AnimationSet{};
	};

	struct SLineModelData
	{
		SStaticModelVertexData VertexData{};
		SLineIndexData IndexData{};
	};

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
		auto LoadStaticModel(STRING Directory, STRING ModelFileName) noexcept->SStaticModelData;

		auto LoadRiggedModel(STRING Directory, STRING ModelFileName) noexcept->SRiggedModelData;

		void LoadAdditionalAnimationIntoRiggedModel(SRiggedModelData& ModelData, STRING Directory, STRING ModelFileName) noexcept;

	private:
		void ExtractNodeTree(const aiScene* Scene, const aiNode* Node, int ParentNodeID, SModelNodeTree& OutNodeTree) noexcept;

		void BuildMeshesAndBonesFromNodes(const STRING Directory, const aiScene* Scene, SRiggedModelData& OutModelData) noexcept;

		void ExtractBone(const aiBone* paiBone, int VertexOffset, SModelBoneTree& OutBoneTree) noexcept;

		void MatchBonesAndVertices(const SModelBoneTree& BoneTree, SRiggedModelVertexData& OutVertexData) noexcept;

		void MatchBonesAndNodes(const SModelBoneTree& BoneTree, SModelNodeTree& OutNodeTree) noexcept;

		void ExtractAnimationSet(const aiScene* Scene, const SModelNodeTree& NodeTree, SModelAnimationSet& OutAnimationSet);
	};
};
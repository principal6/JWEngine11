#pragma once

#include <iostream>
#include <Windows.h>
#include <vector>
#include <memory>
#include <map>
#include <cassert>
#include <crtdbg.h>
#include <chrono>
#include <cstdint>

#include <d3d11.h>
#include <DirectXMath.h> // @IMPORTANT not <xnamath.h> prefix:XM
#include <DirectXPackedVector.h>
#include <d3dcompiler.h>
#include <DirectXTK/pch.h>
#include <wincodec.h> // For GUID formats

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTK.lib")
#pragma comment(lib, "assimp-vc140-mt.lib")

#ifndef CONVENIENT_STD
#define CONVENIENT_STD
	using STRING = std::string;
	using WSTRING = std::wstring;

	template <typename T>
	using VECTOR = std::vector<T>;

	template <typename KeyType, typename ValueType>
	using MAP = std::map<KeyType, ValueType>;

	template <typename T>
	using UNIQUE_PTR = std::unique_ptr<T>;

	template <typename T>
	using SHARED_PTR = std::shared_ptr<T>;

	#define MAKE_UNIQUE(T) std::make_unique<T>
	#define MAKE_SHARED(T) std::make_shared<T>
	#define MAKE_PAIR(Key, Value) std::make_pair(Key, Value)
	#define MOVE(T) std::move(T)
	#define MAKE_UNIQUE_AND_MOVE(T) MOVE(MAKE_UNIQUE(T))
#endif

#ifndef STRING_CONVERTERS
#define STRING_CONVERTERS
	static auto WstringToString(WSTRING Source)->STRING
	{
		STRING Result;

		char* temp = nullptr;
		size_t len = static_cast<size_t>(WideCharToMultiByte(CP_ACP, 0, Source.c_str(), -1, temp, 0, nullptr, nullptr));

		temp = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, Source.c_str(), -1, temp, static_cast<int>(len), nullptr, nullptr);

		Result = temp;

		delete[] temp;
		temp = nullptr;
		return Result;
	}

	static auto StringToWstring(STRING Source)->WSTRING
	{
		WSTRING Result;

		wchar_t* temp = nullptr;
		size_t len = static_cast<size_t>(MultiByteToWideChar(CP_ACP, 0, Source.c_str(), -1, temp, 0));

		temp = new wchar_t[len + 1];
		MultiByteToWideChar(CP_ACP, 0, Source.c_str(), -1, temp, static_cast<int>(len));

		Result = temp;

		delete[] temp;
		temp = nullptr;
		return Result;
	}

	inline auto ConvertIntToSTRING(int value)->STRING
	{
		char temp[255]{};
		sprintf_s(temp, "%d", value);
		return STRING(temp);
	}

	inline auto ConvertFloatToSTRING(float value)->STRING
	{
		char temp[255]{};
		sprintf_s(temp, "%f", value);
		return STRING(temp);
	}

#endif

// Static function prefix
#define STATIC
// Protected method prefix
#define PROTECTED
// Private method prefix
#define PRIVATE

namespace JWEngine
{
	// Forward declaration
	struct SDirectInputDeviceState;

#define JW_AVOID_DUPLICATE_CREATION(Bool) {if (Bool) { return; }}
#define JW_DELETE(pVar) { if(pVar) {delete pVar; pVar = nullptr; }}
#define JW_DELETE_ARRAY(pArray) { if(pArray) {delete[] pArray; pArray = nullptr; }}
#define JW_RELEASE(DxObj) {if (DxObj) { DxObj->Release(); DxObj = nullptr; }}
	
	using namespace DirectX;

	static constexpr uint8_t KMaxBoneCount{ 50 };
	static constexpr uint8_t KColorCountPerTexel{ 4 };
	static constexpr uint8_t KMaxBoneCountPerVertex{ 4 };
	static constexpr uint16_t KMaxFileLength{ 255 };
	static constexpr uint16_t KInputKeyCount{ 256 };
	static constexpr const char* KAssetDirectory{ "Asset\\" };
	static constexpr float KAnimationTickBase{ 30.0f };
	static constexpr XMFLOAT4 KDefaultColorNormals{ XMFLOAT4(0.4f, 0.8f, 0.0f, 1.0f) };
	static constexpr XMFLOAT3 KDefaultColorGrid{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	static constexpr size_t KSizeTInvalid{ MAXSIZE_T };

	inline void JWAbort(const char* Content)
	{
		MessageBoxA(nullptr, Content, "Error", MB_OK);

		std::cout << "[ERROR] " << Content << std::endl << std::endl;

		assert(false);
	}

	enum class EWorldMatrixCalculationOrder
	{
		TransRotScale,
		TransScaleRot,
		RotTransScale,
		RotScaleTrans,
		ScaleTransRot,
		ScaleRotTrans,
	};

	struct SPositionInt
	{
		SPositionInt() {};
		SPositionInt(int _X, int _Y) : X(_X), Y(_Y) {};

		int X{};
		int Y{};
	};

	struct SSizeInt
	{
		SSizeInt() {};
		SSizeInt(uint32_t _Width, uint32_t _Height) : Width(_Width), Height(_Height) {};

		uint32_t Width{};
		uint32_t Height{};
	};

	struct SClearColor
	{
		SClearColor() {};
		SClearColor(float _R, float _G, float _B) : R{ _R }, G{ _G }, B{ _B } {};

		float R{};
		float G{};
		float B{};
	};

	static constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescriptionBase[] =
	{
		{ "POSITION"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"	, 0, DXGI_FORMAT_R32G32_FLOAT	, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"		, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Diffuse
		{ "COLOR"		, 1, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Specular
	};

	static constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescriptionAnim[] =
	{
		{ "POSITION"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"	, 0, DXGI_FORMAT_R32G32_FLOAT	, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"		, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Diffuse
		{ "COLOR"		, 1, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Specular

		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT	, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // int BoneID[4]
		{ "BLENDWEIGHT"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // float Weight[4]
	};
	
	struct SVertexStaticModel
	{
		SVertexStaticModel() {};
		SVertexStaticModel(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SVertexStaticModel(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SVertexStaticModel(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal } {};
		SVertexStaticModel(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse } {};
		SVertexStaticModel(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse, XMFLOAT4 _Specular) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse }, Specular{ _Specular } {};
		SVertexStaticModel(XMFLOAT3 _Position, XMFLOAT4 _ColorDiffuse) : // For drawing model's normals or JWLine
			Position{ _Position }, ColorDiffuse{ _ColorDiffuse } {};
		SVertexStaticModel(float x, float y, float z) :
			Position{ x, y, z } {};
		SVertexStaticModel(float x, float y, float z, float u, float v) :
			Position{ x, y, z }, TextureCoordinates{ u, v } {};
		SVertexStaticModel(float x, float y, float z, float r, float g, float b, float a) :
			Position{ x, y, z }, ColorDiffuse{ r, g, b, a } {};
		SVertexStaticModel(float x, float y, float z, float u, float v, float r, float g, float b, float a) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, ColorDiffuse{ r, g, b, a } {};
		SVertexStaticModel(float x, float y, float z, float u, float v, float nx, float ny, float nz) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz } {};
		SVertexStaticModel(float x, float y, float z, float u, float v, float nx, float ny, float nz, float dr, float dg, float db, float da) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz }, ColorDiffuse{ dr, dg, db, da } {};

		XMFLOAT3 Position{};
		XMFLOAT2 TextureCoordinates{};
		XMFLOAT3 Normal{};
		XMFLOAT4 ColorDiffuse{ 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 Specular{};
	};
	
	struct SVertexRiggedModel
	{
		SVertexRiggedModel() {};
		SVertexRiggedModel(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SVertexRiggedModel(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SVertexRiggedModel(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal } {};
		SVertexRiggedModel(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse } {};
		SVertexRiggedModel(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse, XMFLOAT4 _Specular) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse }, Specular{ _Specular } {};
		SVertexRiggedModel(XMFLOAT3 _Position, XMFLOAT4 _ColorDiffuse) : // For drawing model's normals or JWLine
			Position{ _Position }, ColorDiffuse{ _ColorDiffuse } {};
		SVertexRiggedModel(float x, float y, float z) :
			Position{ x, y, z } {};
		SVertexRiggedModel(float x, float y, float z, float u, float v) :
			Position{ x, y, z }, TextureCoordinates{ u, v } {};
		SVertexRiggedModel(float x, float y, float z, float u, float v, float nx, float ny, float nz) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz } {};
		SVertexRiggedModel(float x, float y, float z, float u, float v, float nx, float ny, float nz, float dr, float dg, float db, float da) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz }, ColorDiffuse{ dr, dg, db, da } {};

		XMFLOAT3 Position{};
		XMFLOAT2 TextureCoordinates{};
		XMFLOAT3 Normal{};
		XMFLOAT4 ColorDiffuse{ 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 Specular{};

		int BoneIndex[KMaxBoneCountPerVertex]{}; // BLENDINDICES
		float BoneWeight[KMaxBoneCountPerVertex]{}; // BLENDWEIGHT

		// From here below, data will NOT be sent to Intput Merger
		int BoneCount{};

		void AddBone(int _BoneIndex, float _BoneWeight)
		{
			if (BoneCount < KMaxBoneCountPerVertex)
			{
				BoneIndex[BoneCount] = _BoneIndex;
				BoneWeight[BoneCount] = _BoneWeight;

				++BoneCount;
			}
		}
	};

	struct SVertexDataStaticModel
	{
		VECTOR<SVertexStaticModel> vVertices;
		UINT Stride{ static_cast<UINT>(sizeof(SVertexStaticModel)) };
		UINT Offset{};

		void Clear() noexcept { vVertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(vVertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SVertexStaticModel)); };
		auto GetPtrData() const noexcept { return &vVertices[0]; };
		auto GetPtrStride() const noexcept { return &Stride; };
		auto GetPtrOffset() const noexcept { return &Offset; };
		void EmptyData() noexcept { memset(&vVertices[0], 0, GetByteSize()); };
	};

	struct SVertexDataRiggedModel
	{
		VECTOR<SVertexRiggedModel> vVertices;
		UINT Stride{ static_cast<UINT>(sizeof(SVertexRiggedModel)) };
		UINT Offset{};

		void Clear() noexcept { vVertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(vVertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SVertexRiggedModel)); };
		auto GetPtrData() const noexcept { return &vVertices[0]; };
		auto GetPtrStride() const noexcept { return &Stride; };
		auto GetPtrOffset() const noexcept { return &Offset; };
		void EmptyData() noexcept { memset(&vVertices[0], 0, GetByteSize()); };
	};

	struct SIndexTriangle
	{
		SIndexTriangle() {};
		SIndexTriangle(DWORD __0, DWORD __1, DWORD __2) : _0{ __0 }, _1{ __1 }, _2{ __2 } {};

		DWORD _0{};
		DWORD _1{};
		DWORD _2{};
	};
	
	struct SIndexLine
	{
		SIndexLine() {};
		SIndexLine(DWORD __0, DWORD __1) : _0{ __0 }, _1{ __1 } {};

		DWORD _0{};
		DWORD _1{};
	};

	struct SIndexDataTriangle
	{
		VECTOR<SIndexTriangle> vIndices;

		void Clear() noexcept { vIndices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(vIndices.size() * 3); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(DWORD)); };
		auto GetPtrData() const noexcept { return &vIndices[0]; };
	};

	struct SIndexDataLine
	{
		VECTOR<SIndexLine> vIndices;

		void Clear() noexcept { vIndices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(vIndices.size() * 2); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(DWORD)); };
		auto GetPtrData() const noexcept { return &vIndices[0]; };
	};

	enum class ERenderType : uint8_t
	{
		Invalid,

		Model_Static,
		Model_Rigged,

		Model_Line3D,
		Model_Line2D,

		Image_2D,
	};

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

	// StaticModel & Image2D
	struct SStaticModelData
	{
		SVertexDataStaticModel VertexData{};
		SIndexDataTriangle IndexData{};

		bool HasTexture{ false };
		WSTRING TextureFileNameW{};
	};

	// RiggedModel
	struct SRiggedModelData
	{
		SVertexDataRiggedModel VertexData{};
		SIndexDataTriangle IndexData{};

		bool HasTexture{ false };
		WSTRING TextureFileNameW{};

		SModelNodeTree NodeTree{};
		SModelBoneTree BoneTree{};
		SModelAnimationSet AnimationSet{};
	};

	// Line2D & Line3D
	struct SLineModelData
	{
		SVertexDataStaticModel VertexData{};
		SIndexDataLine IndexData{};
	};
	
	struct SVSCBSpace
	{
		SVSCBSpace() {};
		SVSCBSpace(XMMATRIX _WVP) : WVP{ _WVP } {};
		SVSCBSpace(XMMATRIX _WVP, XMMATRIX _World) : WVP{ _WVP }, World{ _World } {};

		XMMATRIX	WVP{};
		XMMATRIX	World{};
	};

	struct SVSCBFlags
	{
		SVSCBFlags() {};

		BOOL	ShouldUseGPUAnimation{ FALSE };
		float	pad[3]{};
	};
	
	struct SVSCBCPUAnimation
	{
		SVSCBCPUAnimation() {};

		XMMATRIX	TransformedBoneMatrices[KMaxBoneCount]{};
	};

	struct SVSCBGPUAnimation
	{
		SVSCBGPUAnimation() {};

		// Animation info for GPU
		uint32_t	AnimationID{};
		uint32_t	CurrFrame{};
		uint32_t	NextFrame{};
		float		DeltaTime{};
	};
	
	struct SPSCBFlags
	{
		SPSCBFlags() = default;
		SPSCBFlags(BOOL _HasTexture)
			: HasTexture{ _HasTexture } {};
		SPSCBFlags(BOOL _HasTexture, BOOL _UseLighting)
			: HasTexture{ _HasTexture }, UseLighting{ _UseLighting } {};

		BOOL HasTexture{ FALSE };
		BOOL UseLighting{ FALSE };
		float pad[2]{};
	};

	struct SPSCBLights
	{
		SPSCBLights() = default;
		SPSCBLights(XMFLOAT4 _AmbientLight)
			: AmbientColor{ _AmbientLight } {};
		SPSCBLights(XMFLOAT4 _AmbientLight, XMFLOAT4 _DirectionalColor, XMFLOAT4 _DirectionalDirection)
			: AmbientColor{ _AmbientLight }, DirectionalColor{ _DirectionalColor }, DirectionalDirection{ _DirectionalDirection } {};

		XMFLOAT4 AmbientColor{};

		XMFLOAT4 DirectionalColor{};
		XMFLOAT4 DirectionalDirection{};
	};

	struct SPSCBCamera
	{
		SPSCBCamera() = default;
		SPSCBCamera(XMFLOAT4 _CameraPosition) : CameraPosition{ _CameraPosition } {};

		XMFLOAT4 CameraPosition{};
	};
};

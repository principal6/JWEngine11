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
#define JW_RELEASE(DxObj) {if (DxObj) { DxObj->Release(); DxObj = nullptr; }}
	
	using namespace DirectX;

	static constexpr int KMaxBoneCount{ 50 };
	static constexpr int KMaxBoneCountPerVertex{ 4 };
	static constexpr int KMaxFileLength{ 255 };
	static constexpr int KInputKeyCount{ 256 };
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

	enum class EModelType
	{
		Invalid,
		StaticModel,
		SkinnedModel,
		LineModel,
	};

	enum class EWorldMatrixCalculationOrder
	{
		TransRotScale,
		TransScaleRot,
		RotTransScale,
		RotScaleTrans,
		ScaleTransRot,
		ScaleRotTrans,
	};

	enum class ELightType
	{
		Invalid,

		Ambient, // Entire scene's base light
		Directional, // Sunlight
		Pointlight, // Bulb
		Spotlight, // Flashlight
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
		SSizeInt(int _Width, int _Height) : Width(_Width), Height(_Height) {};

		int Width{};
		int Height{};
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
	
	static constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescriptionColor[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	struct SStaticVertex
	{
		SStaticVertex() {};
		SStaticVertex(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SStaticVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SStaticVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal } {};
		SStaticVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse } {};
		SStaticVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse, XMFLOAT4 _Specular) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse }, Specular{ _Specular } {};
		SStaticVertex(XMFLOAT3 _Position, XMFLOAT4 _ColorDiffuse) : // For drawing model's normals or JWLine
			Position{ _Position }, ColorDiffuse{ _ColorDiffuse } {};
		SStaticVertex(float x, float y, float z) :
			Position{ x, y, z } {};
		SStaticVertex(float x, float y, float z, float u, float v) :
			Position{ x, y, z }, TextureCoordinates{ u, v } {};
		SStaticVertex(float x, float y, float z, float r, float g, float b, float a) :
			Position{ x, y, z }, ColorDiffuse{ r, g, b, a } {};
		SStaticVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz } {};
		SStaticVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, float dr, float dg, float db, float da) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz }, ColorDiffuse{ dr, dg, db, da } {};

		XMFLOAT3 Position{};
		XMFLOAT2 TextureCoordinates{};
		XMFLOAT3 Normal{};
		XMFLOAT4 ColorDiffuse{};
		XMFLOAT4 Specular{};
	};
	
	struct SSkinnedVertex
	{
		SSkinnedVertex() {};
		SSkinnedVertex(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SSkinnedVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SSkinnedVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal } {};
		SSkinnedVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse } {};
		SSkinnedVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse, XMFLOAT4 _Specular) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse }, Specular{ _Specular } {};
		SSkinnedVertex(XMFLOAT3 _Position, XMFLOAT4 _ColorDiffuse) : // For drawing model's normals or JWLine
			Position{ _Position }, ColorDiffuse{ _ColorDiffuse } {};
		SSkinnedVertex(float x, float y, float z) :
			Position{ x, y, z } {};
		SSkinnedVertex(float x, float y, float z, float u, float v) :
			Position{ x, y, z }, TextureCoordinates{ u, v } {};
		SSkinnedVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz } {};
		SSkinnedVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, float dr, float dg, float db, float da) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz }, ColorDiffuse{ dr, dg, db, da } {};

		XMFLOAT3 Position{};
		XMFLOAT2 TextureCoordinates{};
		XMFLOAT3 Normal{};
		XMFLOAT4 ColorDiffuse{};
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

	struct SColorVertex
	{
		SColorVertex() {};
		SColorVertex(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SColorVertex(XMFLOAT3 _Position, XMFLOAT4 _ColorRGBA) :
			Position{ _Position }, ColorRGBA{ _ColorRGBA } {};
		SColorVertex(float x, float y, float z) :
			Position{ x, y, z } {};
		SColorVertex(float x, float y, float z, float r, float g, float b, float a) :
			Position{ x, y, z }, ColorRGBA{ r, g, b, a } {};

		XMFLOAT3 Position{};
		XMFLOAT4 ColorRGBA{ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	struct SStaticVertexData
	{
		VECTOR<SStaticVertex> Vertices;
		UINT Stride{ static_cast<UINT>(sizeof(SStaticVertex)) };
		UINT Offset{};

		void Clear() noexcept { Vertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Vertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SStaticVertex)); };
		auto GetPtrData() const noexcept { return &Vertices[0]; };
		auto GetPtrStride() const noexcept { return &Stride; };
		auto GetPtrOffset() const noexcept { return &Offset; };
		void EmptyData() noexcept { memset(&Vertices[0], 0, GetByteSize()); };
	};

	struct SSkinnedVertexData
	{
		VECTOR<SSkinnedVertex> Vertices;
		UINT Stride{ static_cast<UINT>(sizeof(SSkinnedVertex)) };
		UINT Offset{};

		void Clear() noexcept { Vertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Vertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SSkinnedVertex)); };
		auto GetPtrData() const noexcept { return &Vertices[0]; };
		auto GetPtrStride() const noexcept { return &Stride; };
		auto GetPtrOffset() const noexcept { return &Offset; };
		void EmptyData() noexcept { memset(&Vertices[0], 0, GetByteSize()); };
	};

	struct SColorVertexData
	{
		VECTOR<SColorVertex> Vertices;
		UINT Stride{ static_cast<UINT>(sizeof(SColorVertex)) };
		UINT Offset{};

		void Clear() noexcept { Vertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Vertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SColorVertex)); };
		auto GetPtrData() const noexcept { return &Vertices[0]; };
		auto GetPtrStride() const noexcept { return &Stride; };
		auto GetPtrOffset() const noexcept { return &Offset; };
		void EmptyData() noexcept { memset(&Vertices[0], 0, GetByteSize()); };
	};

	struct SIndex3
	{
		SIndex3() {};
		SIndex3(DWORD __0, DWORD __1, DWORD __2) : _0{ __0 }, _1{ __1 }, _2{ __2 } {};

		DWORD _0{};
		DWORD _1{};
		DWORD _2{};
	};
	
	struct SIndex2
	{
		SIndex2() {};
		SIndex2(DWORD __0, DWORD __1) : _0{ __0 }, _1{ __1 } {};

		DWORD _0{};
		DWORD _1{};
	};

	struct SIndex3Data
	{
		VECTOR<SIndex3> Indices;

		void Clear() noexcept { Indices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Indices.size() * 3); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(DWORD)); };
		auto GetPtrData() const noexcept { return &Indices[0]; };
	};

	struct SIndex2Data
	{
		VECTOR<SIndex2> Indices;

		void Clear() noexcept { Indices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Indices.size() * 2); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(DWORD)); };
		auto GetPtrData() const noexcept { return &Indices[0]; };
	};

	struct SLineData
	{
		XMFLOAT2 StartPosition{};
		XMFLOAT2 Length{};
		XMFLOAT4 Color{};

		SLineData() {};
		SLineData(XMFLOAT2 _StartPosition, XMFLOAT2 _Length, XMFLOAT4 _Color) : StartPosition{ _StartPosition }, Length{ _Length }, Color{ _Color } {};
	};
	
	struct SVSCBColor
	{
		SVSCBColor() {};
		SVSCBColor(XMMATRIX _WVP) : WVP{ _WVP } {};

		XMMATRIX WVP{};
	};
	
	struct SVSCBStatic
	{
		SVSCBStatic() {};
		SVSCBStatic(XMMATRIX _WVP) : WVP{ _WVP } {};
		SVSCBStatic(XMMATRIX _WVP, XMMATRIX _World) : WVP{ _WVP }, World{ _World } {};

		XMMATRIX WVP{};
		XMMATRIX World{};
	};
	
	struct SVSCBSkinned
	{
		SVSCBSkinned() {};

		XMMATRIX WVP{};
		XMMATRIX World{};

		XMMATRIX TransformedBoneMatrices[KMaxBoneCount]{};
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
		SStaticVertexData VertexData{};
		SIndex3Data IndexData{};
		bool HasTexture{ false };
		WSTRING TextureFileNameW{};
	};
	
	struct SSkinnedModelData
	{
		SSkinnedVertexData VertexData{};
		SIndex3Data IndexData{};
		bool HasTexture{ false };
		STRING BaseDirectory{};
		WSTRING TextureFileNameW{};

		bool IsRigged{ false };

		// If no animation is set, CurrentAnimationID is KSizeTInvalid(-1)
		size_t CurrentAnimationID{ KSizeTInvalid };

		// If SetAnimation() is called, CurrentAnimationTick is reset to 0.
		float CurrentAnimationTick{};

		bool ShouldRepeatCurrentAnimation{ true };

		SModelNodeTree NodeTree{};
		SModelBoneTree BoneTree{};
		SModelAnimationSet AnimationSet{};
	};

	struct SModel2Data
	{
		SStaticVertexData VertexData{};
		SIndex2Data IndexData{};
	};

	struct SLightData
	{
		ELightType LightType{ ELightType::Invalid };
		XMFLOAT3 LightColor{}; // Ambient | Directional | Pointlight | Spotlight
		float Intensity{}; // Ambient | Directional | Pointlight | Spotlight
		XMFLOAT3 Position{}; // ( Ambient | Directional ) | Pointlight | Spotlight
		float Range{}; // Pointlight | Spotlight ?? Radius??
		XMFLOAT3 Direction{}; // Directional | Spotlight
		XMFLOAT3 Attenuation{}; // Pointlight | Spotlight
		float Cone{}; // Spotlight

		SLightData() = default;

		// Make AMBIENT light
		// @warning: '_Position' data will only be used for light model's 3D representation,
		// and it has NOTHING to do with ambient light's INTENSITY.
		SLightData(XMFLOAT3 _Color, float _Intensity, XMFLOAT3 _Position)
			: LightType{ ELightType::Ambient }, LightColor{ _Color }, Intensity{ _Intensity }, Position{ _Position } {};

		// Make DIRECTIONAL light
		// @warning: '_Position' data WILL be used to calculate the DIRECTION of directional light.
		SLightData(XMFLOAT3 _Color, XMFLOAT3 _Position, float _Intensity)
			: LightType{ ELightType::Directional }, LightColor{ _Color }, Intensity{ _Intensity }, Position{ _Position } {};

		// Make POINT light
		SLightData(XMFLOAT3 _Color, XMFLOAT3 _Position, float _Intensity, float _Range, XMFLOAT3 _Attenuation)
			: LightType{ ELightType::Directional }, LightColor{ _Color }, Position{ _Position },
			Intensity{ _Intensity }, Range{ _Range }, Attenuation{ _Attenuation } {};
	};

	struct SPSCBDefault
	{
		SPSCBDefault() = default;
		SPSCBDefault(BOOL _HasTexture)
			: HasTexture{ _HasTexture } {};
		SPSCBDefault(BOOL _HasTexture, BOOL _UseLighting)
			: HasTexture{ _HasTexture }, UseLighting{ _UseLighting } {};
		SPSCBDefault(BOOL _HasTexture, BOOL _UseLighting, XMFLOAT4 _AmbientLight)
			: HasTexture{ _HasTexture }, UseLighting{ _UseLighting }, AmbientColor{ _AmbientLight } {};
		SPSCBDefault(BOOL _HasTexture, BOOL _UseLighting, XMFLOAT4 _AmbientLight,
			XMFLOAT4 _DirectionalColor, XMFLOAT4 _DirectionalDirection)
			: HasTexture{ _HasTexture }, UseLighting{ _UseLighting }, AmbientColor{ _AmbientLight },
			DirectionalColor{ _DirectionalColor }, DirectionalDirection{ _DirectionalDirection } {};
		SPSCBDefault(BOOL _HasTexture, BOOL _UseLighting, XMFLOAT4 _AmbientLight,
			XMFLOAT4 _DirectionalColor, XMFLOAT4 _DirectionalDirection, XMFLOAT4 _CameraPosition)
			: HasTexture{ _HasTexture }, UseLighting{ _UseLighting }, AmbientColor{ _AmbientLight },
			DirectionalColor{ _DirectionalColor }, DirectionalDirection{ _DirectionalDirection }, CameraPosition{ _CameraPosition } {};

		BOOL HasTexture{ FALSE };
		BOOL UseLighting{ FALSE };
		float pad[2]{};
		XMFLOAT4 AmbientColor{};
		XMFLOAT4 DirectionalColor{};
		XMFLOAT4 DirectionalDirection{};
		XMFLOAT4 CameraPosition{};
	};
};

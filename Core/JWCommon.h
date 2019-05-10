#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <Windows.h>

#include <d3d11.h>
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

	using STEADY_CLOCK = std::chrono::steady_clock;
	using TIME_POINT = std::chrono::time_point<STEADY_CLOCK>;
	using TIME_UNIT_MS = std::chrono::milliseconds;

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

	inline auto ConvertIntToWSTRING(int value, WSTRING& string)->WSTRING&
	{
		wchar_t temp[255]{};
		swprintf_s(temp, L"%d", value);
		string = temp;
		return string;
	}

	inline auto ConvertLongLongToWSTRING(long long value, WSTRING& string)->WSTRING&
	{
		wchar_t temp[255]{};
		swprintf_s(temp, L"%lld", value);
		string = temp;
		return string;
	}

	inline auto ConvertFloatToWSTRING(float value, WSTRING& string)->WSTRING&
	{
		wchar_t temp[255]{};
		swprintf_s(temp, L"%f", value);
		string = temp;
		return string;
	}

	inline auto ConvertIntToSTRING(int value, STRING& string)->STRING&
	{
		char temp[255]{};
		sprintf_s(temp, "%d", value);
		string = temp;
		return string;
	}

	inline auto ConvertLongLongToSTRING(long long value, STRING& string)->STRING&
	{
		char temp[255]{};
		sprintf_s(temp, "%lld", value);
		string = temp;
		return string;
	}

	inline auto ConvertFloatToSTRING(float value, STRING& string)->STRING&
	{
		char temp[255]{};
		sprintf_s(temp, "%f", value);
		string = temp;
		return string;
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

#define JW_DELETE(pVar) { if(pVar) {delete pVar; pVar = nullptr; }}
#define JW_DELETE_ARRAY(pArray) { if(pArray) {delete[] pArray; pArray = nullptr; }}
#define JW_RELEASE(DxObj) {if (DxObj) { DxObj->Release(); DxObj = nullptr; }}
#define JW_RELEASE_CHECK_REFERENCE_COUNT(DxObj) {if (DxObj) { reference_count = DxObj->Release(); DxObj = nullptr; }}
#define JW_ERROR(text) { MessageBoxA(nullptr, \
	(STRING(typeid(*this).name()) + "::" + __func__ + "() " + text).c_str(), "Error", MB_OK); }
#define JW_ERROR_ABORT(text) { MessageBoxA(nullptr, \
	(STRING(typeid(*this).name()) + "::" + __func__ + "() " + text).c_str(), "Error", MB_OK); abort(); }
#define JW_ERROR_RETURN(text) { MessageBoxA(nullptr, \
	(STRING(typeid(*this).name()) + "::" + __func__ + "() " + text).c_str(), "Error", MB_OK); return; }
#define JW_ERROR_RETURN_THIS(text) { MessageBoxA(nullptr, \
	(STRING(typeid(*this).name()) + "::" + __func__ + "() " + text).c_str(), "Error", MB_OK); return this; }
	
	using namespace DirectX;

	static constexpr uint8_t KVertexBufferCount = 3;
	static constexpr uint8_t KVBIDModel = 0;
	static constexpr uint8_t KVBIDRigging = 1;
	static constexpr uint8_t KVBIDInstancing = 2;
	static constexpr uint8_t KMaxBoneCount{ 50 };
	static constexpr uint8_t KColorCountPerTexel{ 4 };
	static constexpr uint8_t KMaxBoneCountPerVertex{ 4 };
	static constexpr uint16_t KMaxFileLength{ 255 };
	static constexpr uint32_t KMaxInstanceCount{ 1000 };
	static constexpr uint16_t KInputKeyCount{ 256 };
	static constexpr const char* KAssetDirectory{ "Asset\\" };
	static constexpr float KAnimationTickBase{ 30.0f };
	static constexpr XMFLOAT4 KDefaultColorNormals{ XMFLOAT4(0.4f, 0.8f, 0.0f, 1.0f) };
	static constexpr XMFLOAT3 KDefaultColorGrid{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	static constexpr size_t KSizeTInvalid{ MAXSIZE_T };
	static constexpr float KOrthographicNearZ{ 1.0f };
	static constexpr float KOrthographicFarZ{ 100.0f };
	static constexpr float KDefaultBoundingEllipsoidRadius = 1.0f;

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
	
	static constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescriptionText[] =
	{
		{ "POSITION"	, 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"	, 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	static constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescriptionModel[] =
	{
		// Vertex buffer #0 (VertexModel)
		{ "POSITION"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "DIFFUSE"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SPECULAR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 96, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		// Vertex buffer #1 (VertexRigging)
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT , 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // int BoneID[4]
		{ "BLENDWEIGHT"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // float Weight[4]

		// Vertex buffer #2 (Instance buffer)
		{ "INST_WORLD"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INST_WORLD"	, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INST_WORLD"	, 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INST_WORLD"	, 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	struct SVertexText
	{
		SVertexText() {};
		SVertexText(XMFLOAT2 _Position) :
			Position{ _Position } {};
		SVertexText(XMFLOAT2 _Position, XMFLOAT2 _TextureCoordinates) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SVertexText(XMFLOAT2 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT4 _Color) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Color{ _Color } {};
		SVertexText(float x, float y) :
			Position{ x, y } {};
		SVertexText(float x, float y, float u, float v) :
			Position{ x, y }, TextureCoordinates{ u, v } {};
		SVertexText(float x, float y, float u, float v, float r, float g, float b, float a) :
			Position{ x, y }, TextureCoordinates{ u, v }, Color{ r, g, b, a } {};

		XMFLOAT2 Position{};
		XMFLOAT2 TextureCoordinates{};
		XMFLOAT4 Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	};
	
	struct SVertexModel
	{
		SVertexModel() {};
		SVertexModel(float x, float y, float z) : Position{ x, y, z, 1.0f } {};
		SVertexModel(float x, float y, float z, float u, float v) : Position{ x, y, z, 1.0f }, TexCoord{ u, v, 0, 0 } {};
		SVertexModel(float x, float y, float z, float u, float v, float cr, float cg, float cb, float ca) :
			Position{ x, y, z, 1.0f }, TexCoord{ u, v, 0, 0 }, Diffuse{ cr, cg, cb, ca } {};
		SVertexModel(XMVECTOR _Position, XMFLOAT4 _Diffuse) : Position{ _Position }, Diffuse{ _Diffuse } {};
		SVertexModel(XMVECTOR _Position, XMVECTOR _Normal, XMFLOAT4 _Diffuse) : Position{ _Position }, Normal{ _Normal }, Diffuse{ _Diffuse } {};
		SVertexModel(XMVECTOR _Position, XMVECTOR _TexCoord, XMVECTOR _Normal, XMFLOAT4 _Diffuse, XMFLOAT4 _Specular) :
			Position{ _Position }, TexCoord{ _TexCoord }, Normal{ _Normal }, Diffuse{ _Diffuse }, Specular{ _Specular } {};

		XMVECTOR Position{};
		XMVECTOR TexCoord{};
		XMVECTOR Normal{};
		XMVECTOR Tangent{};
		XMVECTOR Bitangent{};
		XMFLOAT4 Diffuse{ 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 Specular{};
	};
	
	struct SVertexRigging
	{
		SVertexRigging() {};

		int		BoneIndex[KMaxBoneCountPerVertex]{}; // BLENDINDICES
		float	BoneWeight[KMaxBoneCountPerVertex]{}; // BLENDWEIGHT

		// From here below, data will NOT be sent to Intput Assembler
		int		BoneCount{};

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

	struct SModelInstanceData
	{
		XMMATRIX World{};
	};

	struct SVertexDataText
	{
		VECTOR<SVertexText> vVertices;
		UINT Stride{ static_cast<UINT>(sizeof(SVertexText)) };
		UINT Offset{};

		void Clear() noexcept { vVertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(vVertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SVertexText)); };
		auto GetPtrData() const noexcept { return &vVertices[0]; };
		auto GetPtrStride() const noexcept { return &Stride; };
		auto GetPtrOffset() const noexcept { return &Offset; };
		void EmptyData() noexcept { memset(&vVertices[0], 0, GetByteSize()); };
	};
	
	struct SVertexDataModel
	{
		VECTOR<SVertexModel>		vVerticesModel;
		VECTOR<SVertexRigging>		vVerticesRigging;
		VECTOR<SModelInstanceData>	vInstances;

		UINT						Strides[KVertexBufferCount]
		{ static_cast<UINT>(sizeof(SVertexModel)), static_cast<UINT>(sizeof(SVertexRigging)), static_cast<UINT>(sizeof(SModelInstanceData)) };
		UINT						Offsets[KVertexBufferCount]{ 0, 0, 0 };

		uint32_t					InstanceCount{};

		void Clear() noexcept 
		{
			vVerticesModel.clear(); 
			vVerticesRigging.clear();

			vInstances.clear();
			InstanceCount = 0;
		};

		void AddVertex(const SVertexModel& Model, bool UseRigging = false) noexcept
		{
			vVerticesModel.emplace_back(Model);
			if (UseRigging) { vVerticesRigging.resize(vVerticesModel.size()); }
		}

		inline void InitializeInstance() noexcept { if (vInstances.size() == 0) { vInstances.resize(KMaxInstanceCount); } }

		inline auto PushInstance() noexcept
		{
			if (InstanceCount < KMaxInstanceCount) { ++InstanceCount; }
			return &vInstances[InstanceCount - 1];
		}

		inline void PopInstance() noexcept { if (InstanceCount) { --InstanceCount; } }

		inline void EraseInstanceAt(uint32_t InstanceID) noexcept
		{
			if (InstanceCount)
			{
				auto& last = vInstances[InstanceCount - 1];
				auto& dest = vInstances[InstanceID];

				dest = last;

				last.World = XMMatrixIdentity();

				PopInstance();
			}
		}

		inline auto GetInstance(uint32_t InstanceID) noexcept
		{
			if (InstanceCount == 0) { JW_ERROR_ABORT("No instances.") };

			InstanceID = min(InstanceID, InstanceCount - 1);
			return &vInstances[InstanceID];
		}

		auto GetVertexCount() const noexcept { return static_cast<UINT>(vVerticesModel.size()); };
		auto GetInstanceCount() const noexcept { return InstanceCount; };

		auto GetVertexModelByteSize() const noexcept { return static_cast<UINT>(GetVertexCount() * sizeof(SVertexModel)); };
		auto GetVertexModelPtrData() const noexcept { return &vVerticesModel[0]; };

		auto GetVertexRiggingByteSize() const noexcept { return static_cast<UINT>(GetVertexCount() * sizeof(SVertexRigging)); };
		auto GetVertexRiggingPtrData() const noexcept { return &vVerticesRigging[0]; };

		// @important ( KMaxInstanceCount )
		auto GetInstanceByteSize() const noexcept { return static_cast<UINT>(KMaxInstanceCount * sizeof(SModelInstanceData)); };
		auto GetInstancePtrData() const noexcept { return &vInstances[0]; };

		auto GetPtrStrides() const noexcept { return Strides; };
		auto GetPtrOffsets() const noexcept { return Offsets; };

		void EmptyData() noexcept 
		{ 
			memset(&vVerticesModel[0], 0, GetVertexModelByteSize()); 
			memset(&vVerticesRigging[0], 0, GetVertexRiggingByteSize());
			memset(&vInstances[0], 0, GetInstanceByteSize());
		};
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
		VECTOR<SIndexTriangle> vFaces;

		void Clear() noexcept { vFaces.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(vFaces.size() * 3); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(DWORD)); };
		auto GetPtrData() const noexcept { return &vFaces[0]; };
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
		Model_Dynamic,
		Model_Rigged,

		Model_Line3D,
		Model_Line2D,

		Image_2D,

		Terrain,
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
		int ID{};
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
		size_t NodeID{};
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

	// StaticModel, DynamicModel, RiggedModel, Image2D
	struct SModelData
	{
		SVertexDataModel VertexData{};
		SIndexDataTriangle IndexData{};

		bool HasTexture{ false };
		WSTRING TextureFileNameW{};

		// Rigging data
		SModelNodeTree NodeTree{};
		SModelBoneTree BoneTree{};
		SModelAnimationSet AnimationSet{};
	};

	struct STextureData
	{
		ID3D11Texture2D*			Texture{};
		ID3D11ShaderResourceView*	TextureSRV{};
		SSizeInt					TextureSize{};
	};

	// Line2D & Line3D
	struct SLineModelData
	{
		SVertexDataModel VertexData{};
		SIndexDataLine IndexData{};
	};

	struct SBoundingEllipsoidData
	{
		SBoundingEllipsoidData() {};
		SBoundingEllipsoidData(float _RadiusX, float _RadiusY, float _RadiusZ, float _OffsetX = 0, float _OffsetY = 0, float _OffsetZ = 0) :
			RadiusX{ _RadiusX }, RadiusY{ _RadiusY }, RadiusZ{ _RadiusZ }, Offset{ _OffsetX, _OffsetY, _OffsetZ, 0.0f } {};

		float		RadiusX{ KDefaultBoundingEllipsoidRadius };
		float		RadiusY{ KDefaultBoundingEllipsoidRadius };
		float		RadiusZ{ KDefaultBoundingEllipsoidRadius };

		// Offset is optional.
		// Default value = (0, 0, 0)
		XMVECTOR	Offset{};

		XMMATRIX	EllipsoidWorld{};
	};
	
	struct STerrainQuadTreeNode
	{
		STerrainQuadTreeNode() {};
		STerrainQuadTreeNode(int32_t _NodeID, int32_t _ParentID = -1) : NodeID{ _NodeID }, ParentID{ _ParentID } {};

		int32_t		NodeID{};

		int32_t		ParentID{ -1 };
		int32_t		ChildrenID[4]{ -1, -1, -1, -1 };

		uint32_t	StartX{};
		uint32_t	StartZ{};
		uint32_t	SizeX{};
		uint32_t	SizeZ{};

		bool				HasMeshes{ false };

		// This value is valid only when 'IsMeshNode' is 'true'
		int32_t				SubBoundingEllipsoidID{ -1 };

		ID3D11Buffer*		VertexBuffer{};
		ID3D11Buffer*		IndexBuffer{};
		SVertexDataModel	VertexData{};
		SIndexDataTriangle	IndexData{};
	};
	
	struct STerrainData
	{
		VECTOR<STerrainQuadTreeNode>	QuadTree{};

		uint32_t						TerrainSizeX{};
		uint32_t						TerrainSizeZ{};
		float							HeightFactor{};
		float							XYSizeFactor{};

		SBoundingEllipsoidData			WholeBoundingEllipsoid{};
		VECTOR<SBoundingEllipsoidData>	SubBoundingEllipsoids{};

		void Destroy()
		{
			if (QuadTree.size())
			{
				for (auto& iter : QuadTree)
				{
					JW_RELEASE(iter.VertexBuffer);
					JW_RELEASE(iter.IndexBuffer);
				}
			}
		}
	};
	
	struct SVSCBSpace
	{
		SVSCBSpace() {};
		SVSCBSpace(XMMATRIX _WVP) : WVP{ _WVP } {};
		SVSCBSpace(XMMATRIX _WVP, XMMATRIX _World) : WVP{ _WVP }, World{ _World } {};

		XMMATRIX	WVP{};
		XMMATRIX	World{};
	};
	
	// @important
	enum EFLAGVS : uint32_t
	{
		JWFlagVS_UseAnimation = 0x01,

		// If this bit is off(zero) then the model will be animated on CPU
		JWFlagVS_AnimateOnGPU = 0x02,

		JWFlagVS_Instanced = 0x04,
	};
	using JWFlagVS = uint32_t;

	struct SVSCBFlags
	{
		SVSCBFlags() {};
		SVSCBFlags(JWFlagVS _FlagVS)
			: FlagVS{ _FlagVS } {};

		JWFlagVS	FlagVS{};
		float		pad[3]{};
	};
	
	struct SVSCBCPUAnimationData
	{
		SVSCBCPUAnimationData() {};

		XMMATRIX	TransformedBoneMatrices[KMaxBoneCount]{};
	};

	// Animation info for GPU
	struct SVSCBGPUAnimationData
	{
		SVSCBGPUAnimationData() {};
		
		uint32_t	AnimationID{};
		uint32_t	CurrFrame{};
		uint32_t	NextFrame{};
		float		DeltaTime{};
	};

	// @important
	enum EFLAGPS : uint32_t
	{
		JWFlagPS_UseLighting		= 0x0001,
		JWFlagPS_UseDiffuseTexture	= 0x0002,
		JWFlagPS_UseNormalTexture	= 0x0004,
	};
	using JWFlagPS = uint32_t;

	struct SPSCBFlags
	{
		SPSCBFlags() {};
		SPSCBFlags(JWFlagPS _FlagPS)
			: FlagPS{ _FlagPS } {};

		JWFlagPS	FlagPS{};
		float		pad[3]{};
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

	class JWLogger
	{
	public:
		void InitializeTime() noexcept
		{
			m_LastTimePoint = m_Clock.now();
		}

		void LogPause(STRING Content) noexcept
		{
			AddLog(Content);
			system("pause");
		}

		void AddLog(STRING Content) noexcept
		{
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_Clock.now() - m_LastTimePoint);
			m_LastTimePoint = m_Clock.now();

			STRING temp{};
			STRING log{};
			log = "[LOG #" + ConvertIntToSTRING(m_LogIndex, temp) + "] " + Content;
			log += "\t( " + ConvertLongLongToSTRING(elapsed_ms.count(), temp) + " ms elapsed. )\n";

			std::cout << log;
			m_LogText.append(log);

			++m_LogIndex;
		}

		void SaveLog(STRING Directory) noexcept
		{
			std::ofstream file;
			file.open(Directory + "LOG.txt");
			file << m_LogText;
			file.close();
		}

	private:
		STRING		m_LogText{};
		uint32_t	m_LogIndex{};

		std::chrono::steady_clock m_Clock{};
		std::chrono::time_point<std::chrono::steady_clock> m_LastTimePoint{};
	};

#define LOG_METHOD() { if (m_pLogger) { m_pLogger->AddLog("===" + STRING(typeid(*this).name()) + "::" + __func__ + "() called ==="); } }
#define LOG_ADD(Content) { if (m_pLogger) { m_pLogger->AddLog(Content); } }
#define LOG_SAVE(BaseDirectory) { if (m_pLogger) { m_pLogger->SaveLog(BaseDirectory); } }
#define LOG_METHOD_FINISH() { if (m_pLogger) { m_pLogger->AddLog("===" + STRING(typeid(*this).name()) + "::" + __func__ + "() finished ==="); } }
};

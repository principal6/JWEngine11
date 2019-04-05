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
	
	static constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescriptionColor[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	struct SStaticModelVertex
	{
		SStaticModelVertex() {};
		SStaticModelVertex(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SStaticModelVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SStaticModelVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal } {};
		SStaticModelVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse } {};
		SStaticModelVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse, XMFLOAT4 _Specular) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse }, Specular{ _Specular } {};
		SStaticModelVertex(XMFLOAT3 _Position, XMFLOAT4 _ColorDiffuse) : // For drawing model's normals or JWLine
			Position{ _Position }, ColorDiffuse{ _ColorDiffuse } {};
		SStaticModelVertex(float x, float y, float z) :
			Position{ x, y, z } {};
		SStaticModelVertex(float x, float y, float z, float u, float v) :
			Position{ x, y, z }, TextureCoordinates{ u, v } {};
		SStaticModelVertex(float x, float y, float z, float r, float g, float b, float a) :
			Position{ x, y, z }, ColorDiffuse{ r, g, b, a } {};
		SStaticModelVertex(float x, float y, float z, float u, float v, float r, float g, float b, float a) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, ColorDiffuse{ r, g, b, a } {};
		SStaticModelVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz } {};
		SStaticModelVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, float dr, float dg, float db, float da) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz }, ColorDiffuse{ dr, dg, db, da } {};

		XMFLOAT3 Position{};
		XMFLOAT2 TextureCoordinates{};
		XMFLOAT3 Normal{};
		XMFLOAT4 ColorDiffuse{ 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 Specular{};
	};
	
	struct SRiggedModelVertex
	{
		SRiggedModelVertex() {};
		SRiggedModelVertex(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SRiggedModelVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SRiggedModelVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal } {};
		SRiggedModelVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse } {};
		SRiggedModelVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse, XMFLOAT4 _Specular) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse }, Specular{ _Specular } {};
		SRiggedModelVertex(XMFLOAT3 _Position, XMFLOAT4 _ColorDiffuse) : // For drawing model's normals or JWLine
			Position{ _Position }, ColorDiffuse{ _ColorDiffuse } {};
		SRiggedModelVertex(float x, float y, float z) :
			Position{ x, y, z } {};
		SRiggedModelVertex(float x, float y, float z, float u, float v) :
			Position{ x, y, z }, TextureCoordinates{ u, v } {};
		SRiggedModelVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz } {};
		SRiggedModelVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, float dr, float dg, float db, float da) :
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
	
	struct SStaticModelVertexData
	{
		VECTOR<SStaticModelVertex> Vertices;
		UINT Stride{ static_cast<UINT>(sizeof(SStaticModelVertex)) };
		UINT Offset{};

		void Clear() noexcept { Vertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Vertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SStaticModelVertex)); };
		auto GetPtrData() const noexcept { return &Vertices[0]; };
		auto GetPtrStride() const noexcept { return &Stride; };
		auto GetPtrOffset() const noexcept { return &Offset; };
		void EmptyData() noexcept { memset(&Vertices[0], 0, GetByteSize()); };
	};

	struct SRiggedModelVertexData
	{
		VECTOR<SRiggedModelVertex> Vertices;
		UINT Stride{ static_cast<UINT>(sizeof(SRiggedModelVertex)) };
		UINT Offset{};

		void Clear() noexcept { Vertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Vertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SRiggedModelVertex)); };
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

	struct SModelIndexData
	{
		VECTOR<SIndex3> Indices;

		void Clear() noexcept { Indices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Indices.size() * 3); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(DWORD)); };
		auto GetPtrData() const noexcept { return &Indices[0]; };
	};

	struct SLineIndexData
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
	
	struct SVSCBRigged
	{
		SVSCBRigged() {};

		XMMATRIX WVP{};
		XMMATRIX World{};

		XMMATRIX TransformedBoneMatrices[KMaxBoneCount]{};
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

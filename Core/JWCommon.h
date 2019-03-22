#pragma once

#include <iostream>
#include <Windows.h>
#include <vector>
#include <memory>
#include <map>
#include <cassert>
#include <crtdbg.h>
#include <chrono>

#include <d3d11.h>
#include <DirectXMath.h> // @IMPORTANT not <xnamath.h> prefix:XM
#include <DirectXPackedVector.h>
#include <d3dcompiler.h>
#include "../DirectXTK/Inc/pch.h"

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

	using FP_ON_INPUT = void(*)(SDirectInputDeviceState&);
	using FP_ON_RENDER = void(*)(void);
#define JW_FUNCTION_ON_INPUT(FunctionName) void FunctionName(SDirectInputDeviceState& DeviceState)
#define JW_FUNCTION_ON_RENDER(FunctionName) void FunctionName()
	
	using namespace DirectX;
	
	static constexpr int KMaxFileLength{ 255 };
	static constexpr int KInputKeyCount{ 256 };
	static constexpr const char* KAssetDirectory{ "Asset\\" };
	static constexpr XMFLOAT4 KDefaultColorNormals{ XMFLOAT4(0.4f, 0.8f, 0.0f, 1.0f) };
	static constexpr XMFLOAT3 KDefaultColorGrid{ XMFLOAT3(1.0f, 1.0f, 1.0f) };

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

		// Make ambient light
		// @warning: '_Position' data will only used for light model's representation
		// it has nothing to do with ambient light's intensity.
		SLightData(XMFLOAT3 _Color, XMFLOAT3 _Position, float _Intensity)
			: LightType{ ELightType::Ambient }, LightColor{ _Color }, Position{ _Position },
			Intensity{ _Intensity } {};
		
		// Make directional light
		SLightData(XMFLOAT3 _Color, XMFLOAT3 _Position, float _Intensity, XMFLOAT3 _Direction)
			: LightType{ ELightType::Directional }, LightColor{ _Color }, Position{ _Position },
			Intensity{ _Intensity }, Direction{ _Direction } {};

		// Make point light
		SLightData(XMFLOAT3 _Color, XMFLOAT3 _Position, float _Intensity, float _Range, XMFLOAT3 _Attenuation)
			: LightType{ ELightType::Directional }, LightColor{ _Color }, Position{ _Position },
			Intensity{ _Intensity }, Range{ _Range }, Attenuation{ _Attenuation } {};
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

	static constexpr D3D11_INPUT_ELEMENT_DESC KInputElementDescription[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT	, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	static constexpr UINT KInputElementSize = ARRAYSIZE(KInputElementDescription);

	static constexpr D3D11_INPUT_ELEMENT_DESC KInputElementColorDescription[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	static constexpr UINT KInputElementColorSize = ARRAYSIZE(KInputElementColorDescription);
	
	struct SVertex
	{
		SVertex() {};
		SVertex(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal } {};
		SVertex(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates, XMFLOAT3 _Normal, XMFLOAT4 _ColorDiffuse) :
			Position{ _Position }, TextureCoordinates{ _TextureCoordinates }, Normal{ _Normal }, ColorDiffuse{ _ColorDiffuse } {};
		SVertex(XMFLOAT3 _Position, XMFLOAT4 _ColorDiffuse) : // For drawing model's normals or JWLine
			Position{ _Position }, ColorDiffuse{ _ColorDiffuse } {};
		SVertex(float x, float y, float z) :
			Position{ x, y, z } {};
		SVertex(float x, float y, float z, float u, float v) :
			Position{ x, y, z }, TextureCoordinates{ u, v } {};
		SVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz } {};
		SVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, float dr, float dg, float db, float da) :
			Position{ x, y, z }, TextureCoordinates{ u, v }, Normal{ nx, ny, nz }, ColorDiffuse{ dr, dg, db, da } {};

		XMFLOAT3 Position{};
		XMFLOAT2 TextureCoordinates{};
		XMFLOAT3 Normal{};
		XMFLOAT4 ColorDiffuse{};
	};

	struct SVertexColor
	{
		SVertexColor() {};
		SVertexColor(XMFLOAT3 _Position) :
			Position{ _Position } {};
		SVertexColor(XMFLOAT3 _Position, XMFLOAT4 _ColorRGBA) :
			Position{ _Position }, ColorRGBA{ _ColorRGBA } {};
		SVertexColor(float x, float y, float z) :
			Position{ x, y, z } {};
		SVertexColor(float x, float y, float z, float r, float g, float b, float a) :
			Position{ x, y, z }, ColorRGBA{ r, g, b, a } {};

		XMFLOAT3 Position{};
		XMFLOAT4 ColorRGBA{ 1.0f, 1.0f, 1.0f, 1.0f };
	};
	
	struct SVertexData
	{
		VECTOR<SVertex> Vertices;
		UINT Stride{ static_cast<UINT>(sizeof(SVertex)) };
		UINT Offset{};

		void Clear() noexcept { Vertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Vertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SVertex)); };
		auto GetPtrData() const noexcept { return &Vertices[0]; };
		auto GetPtrStride() const noexcept { return &Stride; };
		auto GetPtrOffset() const noexcept { return &Offset; };
		void EmptyData() noexcept { memset(&Vertices[0], 0, GetByteSize()); };
	};

	struct SVertexColorData
	{
		VECTOR<SVertexColor> Vertices;
		UINT Stride{ static_cast<UINT>(sizeof(SVertexColor)) };
		UINT Offset{};

		void Clear() noexcept { Vertices.clear(); };
		auto GetCount() const noexcept { return static_cast<UINT>(Vertices.size()); };
		auto GetByteSize() const noexcept { return static_cast<UINT>(GetCount() * sizeof(SVertexColor)); };
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

	struct SModelData
	{
		SVertexData VertexData{};
		SIndex3Data IndexData{};
		bool HasTexture{ false };
		WSTRING TextureFileNameW{};
	};

	struct SModel2Data
	{
		SVertexData VertexData{};
		SIndex2Data IndexData{};
	};

	struct SLineData
	{
		XMFLOAT2 StartPosition{};
		XMFLOAT2 Length{};
		XMFLOAT4 Color{};

		SLineData() {};
		SLineData(XMFLOAT2 _StartPosition, XMFLOAT2 _Length, XMFLOAT4 _Color) : StartPosition{ _StartPosition }, Length{ _Length }, Color{ _Color } {};
	};

	struct SColorVSConstantBufferData
	{
		SColorVSConstantBufferData() {};
		SColorVSConstantBufferData(XMMATRIX _WVP) : WVP{ _WVP } {};

		XMMATRIX WVP{};
	};

	struct SDefaultVSCBDefault
	{
		SDefaultVSCBDefault() {};
		SDefaultVSCBDefault(XMMATRIX _WVP) : WVP{ _WVP } {};
		SDefaultVSCBDefault(XMMATRIX _WVP, XMMATRIX _World) : WVP{ _WVP }, World{ _World } {};

		XMMATRIX WVP{};
		XMMATRIX World{};
	};

	struct SDefaultPSCBDefault
	{
		SDefaultPSCBDefault() {};
		SDefaultPSCBDefault(BOOL _HasTexture)
			: HasTexture{ _HasTexture } {};
		SDefaultPSCBDefault(BOOL _HasTexture, BOOL _UseLighting)
			: HasTexture{ _HasTexture }, UseLighting{ _UseLighting } {};
		SDefaultPSCBDefault(BOOL _HasTexture, BOOL _UseLighting, XMFLOAT4 _AmbientLight)
			: HasTexture{ _HasTexture }, UseLighting{ _UseLighting }, AmbientLight{ _AmbientLight } {};

		BOOL HasTexture{ FALSE };
		BOOL UseLighting{ FALSE };
		float pad[2]{};
		XMFLOAT4 AmbientLight{};
	};
	
	inline void JWAbort(const char* Content)
	{
		MessageBoxA(nullptr, Content, "Error", MB_OK);

		std::cout << "[ERROR] " << Content << std::endl << std::endl;

		assert(false);
	}
};

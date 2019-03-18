#pragma once

#include <iostream>
#include <Windows.h>
#include <vector>
#include <memory>
#include <map>
#include <cassert>
#include <crtdbg.h>

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
#endif

// Static function prefix
#define STATIC
// Protected method prefix
#define PROTECTED
// Private method prefix
#define PRIVATE

#define AVOID_DUPLICATE_CREATION(BOOL) {if(BOOL){return;}}

#define JW_RELEASE(DxObj) {if (DxObj) { DxObj->Release(); DxObj = nullptr; }}

namespace JWEngine
{
	using namespace DirectX;

	static constexpr int MAX_FILE_LENGTH = 255;

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
	
	struct SVertexTexture
	{
		SVertexTexture() {};
		SVertexTexture(XMFLOAT3 _Position) : Position{ _Position } {};
		SVertexTexture(XMFLOAT3 _Position, XMFLOAT2 _TextureCoordinates) : Position{ _Position }, TextureCoordinates{ _TextureCoordinates } {};
		SVertexTexture(float x, float y, float z, float u, float v) : Position{ x, y, z }, TextureCoordinates{ u, v } {};
		
		XMFLOAT3 Position{};
		XMFLOAT2 TextureCoordinates{};
	};

	struct SVertexColor
	{
		SVertexColor() {};
		SVertexColor(XMFLOAT3 _Position) : Position{ _Position } {};
		SVertexColor(XMFLOAT3 _Position, XMFLOAT4 _Color) : Position{ _Position }, Color{ _Color } {};
		SVertexColor(float x, float y, float z, float a, float r, float g, float b) : Position{ x, y, z }, Color{ r, g, b, a } {};

		XMFLOAT3 Position{};
		XMFLOAT4 Color{};
	};

	static constexpr D3D11_INPUT_ELEMENT_DESC InputElementDescriptionColor[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	static constexpr D3D11_INPUT_ELEMENT_DESC InputElementDescriptionTexture[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	#define USE_TEXTURE
	#ifdef USE_TEXTURE
		using SVertex = SVertexTexture;
		#define INPUT_ELEMENT_DESCRIPTION InputElementDescriptionTexture
	#else
		using SVertex = SVertexColor;
		#define INPUT_ELEMENT_DESCRIPTION InputElementDescriptionColor
	#endif

	static constexpr UINT InputElementSize = ARRAYSIZE(INPUT_ELEMENT_DESCRIPTION);

	struct SVertexData
	{
		VECTOR<SVertex> Vertices;
		UINT Count{};
	};

	struct SIndex
	{
		DWORD _0{};
		DWORD _1{};
		DWORD _2{};

		SIndex() {};
		SIndex(DWORD __0, DWORD __1, DWORD __2) : _0{ __0 }, _1{ __1 }, _2{ __2 } {};
	};

	struct SIndexData
	{
		VECTOR<SIndex> Indices;
		UINT Count{};
	};

	struct SConstantBufferDataPerObject
	{
		XMMATRIX WVP{};

		SConstantBufferDataPerObject() {};
		SConstantBufferDataPerObject(XMMATRIX _WVP) : WVP{ _WVP } {};
	};

	inline void JWAbort(const char* Content)
	{
		MessageBoxA(nullptr, Content, "Error", MB_OK);

		std::cout << "[ERROR] " << Content << std::endl << std::endl;

		assert(false);
	}
};

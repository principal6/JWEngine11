#pragma once

#include "../Core/JWModel.h"
#include "../Core/JWDX.h"
#include "../Core/JWCamera.h"

namespace JWEngine
{
	using FP_RENDER = void(*)(void);

	class JWGame
	{
	public:
		JWGame() = default;
		~JWGame() = default;

		void Create(int Width, int Height, STRING Title, STRING Directory) noexcept;

		void SetRenderFunction(FP_RENDER Render) noexcept;
		void SetRasterizerState(ERasterizerState State) noexcept;

		void AddOpaqueModel(STRING Directory, STRING ModelFileName) noexcept;
		auto GetOpaqueModel(size_t ModelIndex) const noexcept->JWModel&;

		void AddTransparentModel(STRING Directory, STRING TransparentModelFileName) noexcept;
		auto GetTransparentModel(size_t TransparentModelIndex) const noexcept->JWModel&;

		auto GetCameraObject() noexcept->JWCamera&;

		void Run() noexcept;

		void DrawAllModels() noexcept;

	private:
		inline auto GetFileNameWithBaseDirectory(STRING& FileName) noexcept->STRING;

		void CheckValidity() const noexcept;

		void SetBlendState(EBlendState State) noexcept;

		void DrawAllOpaqueModels() const noexcept;
		void DrawAllTransparentModels() const noexcept;

	private:
		bool m_IsWindowCreated{ false };
		bool m_IsDXCreated{ false };
		
		STRING m_BaseDirectory;
		SClearColor m_ClearColor{};

		JWWin32Window m_Window{};
		JWDX m_DX{};

		FP_RENDER m_fpRender{};

		JWCamera m_Camera{};

		VECTOR<UNIQUE_PTR<JWModel>> m_pOpaqueModels;
		VECTOR<UNIQUE_PTR<JWModel>> m_pTransparentModels;
	};
};
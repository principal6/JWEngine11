#pragma once

#include "../Core/JWWin32Window.h"
#include "../Core/JWDX.h"
#include "../Core/JWCamera.h"
#include "../Core/JWModel.h"
#include "../Core/JWImage.h"

namespace JWEngine
{
	using FP_RENDER = void(*)(void);

	class JWGame
	{
	public:
		JWGame() = default;
		~JWGame() = default;

		void Create(SPositionInt WindowPosition, SSizeInt WindowSize, STRING Title, STRING Directory) noexcept;

		void SetRenderFunction(FP_RENDER Render) noexcept;
		void SetRasterizerState(ERasterizerState State) noexcept;

		void AddOpaqueModel(STRING Directory, STRING ModelFileName) noexcept;
		auto GetOpaqueModel(size_t ModelIndex) const noexcept->JWModel&;

		void AddTransparentModel(STRING Directory, STRING TransparentModelFileName) noexcept;
		auto GetTransparentModel(size_t TransparentModelIndex) const noexcept->JWModel&;

		void AddImage(STRING Directory, STRING ImageFileName) noexcept;
		auto GetImage(size_t Image2DIndex) const noexcept->JWImage&;

		auto GetCameraObject() noexcept->JWCamera&;

		void Run() noexcept;

		void DrawAll() noexcept;

	private:
		inline auto GetFileNameWithBaseDirectory(const STRING& FileName) const noexcept->STRING;

		void CheckValidity() const noexcept;

		void SetBlendState(EBlendState State) noexcept;
		void SetDepthStencilState(EDepthStencilState State) noexcept;

		void DrawAllOpaqueModels() const noexcept;
		void DrawAllTransparentModels() const noexcept;
		void DrawAll2DImages() const noexcept;

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
		VECTOR<UNIQUE_PTR<JWImage>> m_p2DImages;
	};
};
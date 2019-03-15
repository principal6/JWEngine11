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

		void Create(int Width, int Height, STRING Title) noexcept;

		void SetRenderFunction(FP_RENDER Render) noexcept;
		void SetRasterizerState(ERasterizerState State) noexcept;
		void SetBlendState(EBlendState State) noexcept;

		auto GetCameraObject() noexcept->JWCamera&;
		auto GetModelObject() noexcept->JWModel&;

		void Run() noexcept;

	private:
		void CheckValidity() const noexcept;

	private:
		bool m_IsWindowCreated{ false };
		bool m_IsDXCreated{ false };
		
		SClearColor m_ClearColor{};

		JWWin32Window m_Window{};
		JWDX m_DX{};

		FP_RENDER m_fpRender{};

		JWCamera m_Camera{};
		JWModel m_Mesh{};
	};
};
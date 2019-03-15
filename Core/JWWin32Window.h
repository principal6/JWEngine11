#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWWin32Window
	{
	public:
		JWWin32Window() = default;
		~JWWin32Window() = default;

		void Create(int Width, int Height, const STRING& Title) noexcept;

		auto GetWidth() const noexcept->int;
		auto GetHeight() const noexcept->int;
		auto GethWnd() const noexcept->HWND;
		auto GethInstance() const noexcept->HINSTANCE;

	private:
		HINSTANCE m_hInstance{};
		HWND m_hWnd{};

		SSizeInt m_WindowSize;
	};
};
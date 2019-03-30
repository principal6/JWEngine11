#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	using FP_ON_WINDOWS_KEYDOWN = void(*)(WPARAM&);
#define JW_FUNCTION_ON_WINDOWS_KEYDOWN(FunctionName) void FunctionName(WPARAM& VK)

	class JWWin32Window
	{
	public:
		JWWin32Window() = default;
		~JWWin32Window() = default;

		void Create(SPositionInt Position, SSizeInt Size, const STRING& Title) noexcept;

		void SetOnWindowsKeyDownFunction(FP_ON_WINDOWS_KEYDOWN Function) noexcept;

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
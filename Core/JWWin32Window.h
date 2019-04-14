#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	using FP_ON_WINDOWS_KEY_DOWN = void(*)(WPARAM);
#define JW_FUNCTION_ON_WINDOWS_KEY_DOWN(FunctionName) void FunctionName(WPARAM VK)

	using FP_ON_WINDOWS_CHAR_INPUT = void(*)(TCHAR);
#define JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(FunctionName) void FunctionName(TCHAR Character)

	class JWWin32Window
	{
	public:
		JWWin32Window() = default;
		~JWWin32Window() = default;

		void Create(SPositionInt Position, SSizeInt Size, const STRING& Title) noexcept;

		void SetOnWindowsKeyDownFunction(FP_ON_WINDOWS_KEY_DOWN Function) noexcept;
		void SetOnWindowsCharInputFunction(FP_ON_WINDOWS_CHAR_INPUT Function) noexcept;

		auto GetWidth() const noexcept->int;
		auto GetHeight() const noexcept->int;
		auto GethWnd() const noexcept->HWND;
		auto GethInstance() const noexcept->HINSTANCE;

	private:
		HINSTANCE	m_hInstance{};
		HWND		m_hWnd{};
		WNDCLASSEXA	m_WindowClass{};

		SSizeInt	m_WindowSize;
	};
};
#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	static constexpr DWORD KGameWindowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	using FP_ON_WINDOWS_KEY_DOWN = void(*)(WPARAM);
#define JW_FUNCTION_ON_WINDOWS_KEY_DOWN(FunctionName) void FunctionName(WPARAM VK)

	using FP_ON_WINDOWS_CHAR_INPUT = void(*)(TCHAR);
#define JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(FunctionName) void FunctionName(TCHAR Character)

	using FP_ON_WINDOWS_RESIZE = void(*)(HWND);
#define JW_FUNCTION_ON_WINDOWS_RESIZE(FunctionName) void FunctionName(HWND hWnd)

	class JWWin32Window
	{
	public:
		JWWin32Window() = default;
		~JWWin32Window() = default;

		void Create(const SPosition2& Position, const SSize2& Size, const STRING& Title) noexcept;
		void Destroy() noexcept {};

		void SetOnWindowsKeyDownFunction(FP_ON_WINDOWS_KEY_DOWN Function) noexcept;
		void SetOnWindowsCharInputFunction(FP_ON_WINDOWS_CHAR_INPUT Function) noexcept;
		void SetOnWindowsResizeFunction(FP_ON_WINDOWS_RESIZE Function) noexcept;

		auto GethWnd() const noexcept { return m_hWnd; }
		auto GethInstance() const noexcept { return m_hInstance; }

	private:
		HINSTANCE	m_hInstance{};
		HWND		m_hWnd{};
		WNDCLASSEXA	m_WindowClass{};
	};
};
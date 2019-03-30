#include "JWWin32Window.h"

using namespace JWEngine;

static FP_ON_WINDOWS_KEYDOWN s_fpWindowsKeyDown{};

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (s_fpWindowsKeyDown)
		{
			s_fpWindowsKeyDown(wParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void JWWin32Window::Create(SPositionInt Position, SSizeInt Size, const STRING& Title) noexcept
{
	m_hInstance = GetModuleHandleA(nullptr);

	m_WindowSize.Width = Size.Width;
	m_WindowSize.Height = Size.Height;

	WNDCLASSEXA wc{};
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
	wc.hCursor = nullptr; //LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = nullptr; //(HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "JWEngine";
	wc.hIconSm = LoadIconA(nullptr, IDI_APPLICATION);

	if (!RegisterClassExA(&wc))
	{
		JWAbort("RegisterClassExA() failed.");
	}
	
	RECT rect = { Position.X, Position.Y, Position.X + Size.Width, Position.Y + Size.Height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	m_hWnd = CreateWindowExA(0, wc.lpszClassName, Title.c_str(), WS_OVERLAPPEDWINDOW, Position.X, Position.Y,
		Size.Width, Size.Height, nullptr, nullptr, m_hInstance, nullptr);

	if (!m_hWnd)
	{
		JWAbort("CreateWindowExA() failed.");
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
}

void JWWin32Window::SetOnWindowsKeyDownFunction(FP_ON_WINDOWS_KEYDOWN Function) noexcept
{
	s_fpWindowsKeyDown = Function;
}

auto JWWin32Window::GetWidth() const noexcept->int
{
	return m_WindowSize.Width;
}

auto JWWin32Window::GetHeight() const noexcept->int
{
	return m_WindowSize.Height;
}

auto JWWin32Window::GethWnd() const noexcept->HWND
{
	return m_hWnd;
}

auto JWWin32Window::GethInstance() const noexcept->HINSTANCE
{
	return m_hInstance;
}
#include "JWWin32Window.h"

using namespace JWEngine;

static FP_ON_WINDOWS_KEY_DOWN s_fpWindowsKeyDown{};
static FP_ON_WINDOWS_CHAR_INPUT s_fpWindowsCharKeyPressed{};

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CHAR:
		if (s_fpWindowsCharKeyPressed)
		{
			s_fpWindowsCharKeyPressed(static_cast<TCHAR>(wParam));
		}
		break;
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

	m_WindowClass.cbSize = sizeof(WNDCLASSEXA);
	m_WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	m_WindowClass.lpfnWndProc = WndProc;
	m_WindowClass.cbClsExtra = 0;
	m_WindowClass.cbWndExtra = 0;
	m_WindowClass.hInstance = m_hInstance;
	m_WindowClass.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
	m_WindowClass.hCursor = nullptr; //LoadCursor(NULL, IDC_ARROW);
	m_WindowClass.hbrBackground = nullptr; //(HBRUSH)(COLOR_WINDOW + 2);
	m_WindowClass.lpszMenuName = nullptr;
	m_WindowClass.lpszClassName = "JWEngine";
	m_WindowClass.hIconSm = LoadIconA(nullptr, IDI_APPLICATION);

	assert(RegisterClassExA(&m_WindowClass));	
	
	RECT rect{};
	rect.left = Position.X;
	rect.top = Position.Y;
	rect.right = Position.X + Size.Width;
	rect.bottom = Position.Y + Size.Height;

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	m_hWnd = CreateWindowExA(0, m_WindowClass.lpszClassName, Title.c_str(), WS_OVERLAPPEDWINDOW, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, m_hInstance, nullptr);
	
	assert(m_hWnd);
	
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
}

void JWWin32Window::SetOnWindowsKeyDownFunction(FP_ON_WINDOWS_KEY_DOWN Function) noexcept
{
	s_fpWindowsKeyDown = Function;
}

void JWWin32Window::SetOnWindowsCharInputFunction(FP_ON_WINDOWS_CHAR_INPUT Function) noexcept
{
	s_fpWindowsCharKeyPressed = Function;
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